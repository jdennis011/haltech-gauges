// The online copy of the hub's page, on Cloudflare. The page is the same file the
// hub serves; here it runs in demo mode (no hub). Three things live in this Worker:
//
// 1. Static data behind the same /api paths the hub uses (templates, the channel
//    list, fonts), so the page's code is identical.
// 2. Sign in with Google: the page posts Google's ID token, the Worker verifies it
//    against Google's keys and sets a signed session cookie.
// 3. Per-user storage in KV, behind the hub's own paths for configs, images,
//    planned gauges, assignments and recordings, so what you make follows your
//    account across devices. Signed out, the page keeps its work in the browser instead.

const STATIC_API = {
  '/api/templates': '/data/templates.json',
  '/api/channels/meta': '/data/channels.json',
  '/api/fonts': '/data/fonts.json',
  '/api/library/recordings': '/data/recordings.json',
};

const NAME_RE = /^[A-Za-z0-9_-]{1,32}$/;
const MAC_RE = /^[0-9A-F]{2}(:[0-9A-F]{2}){5}$/;
const COOKIE = 'hg_session';
const SESSION_SECONDS = 30 * 24 * 3600;
const MAX_CONFIG_BYTES = 64 * 1024;
const MAX_RECORDING_BYTES = 1024 * 1024;
const MAX_CONFIGS = 64;
const MAX_RECORDINGS = 20;
const MAX_PLANNED = 8;
const MAX_IMAGE_BYTES = 256 * 1024;
const MAX_IMAGES = 32;
const ACCOUNT_IMAGE_BYTES = 4 * 1024 * 1024;

export default {
  async fetch(request, env) {
    const url = new URL(request.url);
    const path = url.pathname;

    // One name for the site: www goes to the bare domain.
    if (url.hostname === 'www.oagauge.online') { url.hostname = 'oagauge.online'; return Response.redirect(url.toString(), 301); }

    let target = STATIC_API[path];
    if (!target && path.startsWith('/api/templates/')) {
      const name = path.slice('/api/templates/'.length);
      if (NAME_RE.test(name)) target = '/data/templates/' + name + '.json';
    }
    if (!target && path.startsWith('/api/library/recordings/')) {
      const name = path.slice('/api/library/recordings/'.length);
      if (NAME_RE.test(name)) target = '/data/recordings/' + name + '.json';
    }
    if (target) {
      const res = await env.ASSETS.fetch(new URL(target, url));
      if (!res.ok) return json({ ok: false, error: 'not found' }, 404);
      // Static data is public, and a hub's own page may fetch the library across origins.
      return new Response(res.body, { status: 200, headers: { 'content-type': 'application/json; charset=utf-8', 'cache-control': 'public, max-age=300', 'access-control-allow-origin': '*' } });
    }

    if (path === '/api/me' || path.startsWith('/api/auth/')) return auth(request, env, path);

    if (path.startsWith('/api/')) {
      const user = await sessionUser(request, env);
      const handled = await userApi(request, env, path, user);
      if (handled) return handled;
      return json({ ok: false, error: 'this is the online copy: there is no hub behind it' }, 404);
    }
    return env.ASSETS.fetch(request);
  },
};

// ----------------------------------------------------------------- helpers

const json = (data, status = 200, headers = {}) =>
  new Response(JSON.stringify(data), { status, headers: { 'content-type': 'application/json; charset=utf-8', 'cache-control': 'no-store', ...headers } });

const b64url = bytes => btoa(String.fromCharCode(...new Uint8Array(bytes))).replace(/\+/g, '-').replace(/\//g, '_').replace(/=+$/, '');
const b64urlDecode = s => Uint8Array.from(atob(s.replace(/-/g, '+').replace(/_/g, '/').padEnd(Math.ceil(s.length / 4) * 4, '=')), c => c.charCodeAt(0));
const utf8 = s => new TextEncoder().encode(s);
const fromUtf8 = b => new TextDecoder().decode(b);

async function hmac(secret, text) {
  const key = await crypto.subtle.importKey('raw', utf8(secret), { name: 'HMAC', hash: 'SHA-256' }, false, ['sign']);
  return b64url(await crypto.subtle.sign('HMAC', key, utf8(text)));
}

function sameString(a, b) {
  if (a.length !== b.length) return false;
  let diff = 0;
  for (let i = 0; i < a.length; i++) diff |= a.charCodeAt(i) ^ b.charCodeAt(i);
  return diff === 0;
}

function cookies(request) {
  const out = {};
  for (const part of (request.headers.get('cookie') || '').split(';')) {
    const i = part.indexOf('=');
    if (i > 0) out[part.slice(0, i).trim()] = part.slice(i + 1).trim();
  }
  return out;
}

// CRC32 as the hub computes it, so the page's "config in sync" check means the same.
let crcTable = null;
function crc32Bytes(bytes) {
  if (!crcTable) {
    crcTable = new Uint32Array(256);
    for (let n = 0; n < 256; n++) { let c = n; for (let k = 0; k < 8; k++) c = c & 1 ? 0xEDB88320 ^ (c >>> 1) : c >>> 1; crcTable[n] = c >>> 0; }
  }
  let crc = 0xFFFFFFFF;
  for (const b of bytes) crc = crcTable[(crc ^ b) & 0xFF] ^ (crc >>> 8);
  return ((crc ^ 0xFFFFFFFF) >>> 0).toString(16).toUpperCase().padStart(8, '0');
}
const crc32 = text => crc32Bytes(utf8(text));

// The type and size of an image from its header, checked as the hub checks
// them: baseline JPEG or any PNG, at most 466 x 466. {error} if not.
function probeImage(b) {
  const n = b.length, be16 = i => (b[i] << 8) | b[i + 1];
  let type = '', width = 0, height = 0;
  if (n < 8) return { error: 'the file is too short to be an image' };
  if (b[0] === 0xFF && b[1] === 0xD8 && b[2] === 0xFF) {
    for (let pos = 2; pos + 4 <= n;) {
      if (b[pos] !== 0xFF) break;
      const m = b[pos + 1];
      if (m === 0xFF) { pos++; continue; }
      if (m === 0xD8 || m === 0x01 || (m >= 0xD0 && m <= 0xD7)) { pos += 2; continue; }
      if (m === 0xD9 || m === 0xDA) break;
      const len = be16(pos + 2);
      if (len < 2) break;
      if (m >= 0xC0 && m <= 0xCF && m !== 0xC4 && m !== 0xC8 && m !== 0xCC) {
        if ([0xC2, 0xC6, 0xCA, 0xCE].includes(m)) return { error: 'progressive JPEG: save it as a baseline (standard) JPEG' };
        if (m !== 0xC0) return { error: 'this kind of JPEG (lossless or arithmetic-coded) is not supported: save it as a baseline JPEG' };
        if (pos + 10 > n) break;
        if (b[pos + 4] !== 8) return { error: 'JPEG with 12-bit samples is not supported' };
        if (b[pos + 9] !== 1 && b[pos + 9] !== 3) return { error: 'JPEG in CMYK is not supported: save it as RGB' };
        type = 'jpeg'; height = be16(pos + 5); width = be16(pos + 7);
        break;
      }
      pos += 2 + len;
    }
    if (!type) return { error: 'the JPEG is damaged or cut short' };
  } else if ([0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A].every((v, i) => b[i] === v)) {
    if (n < 33 || String.fromCharCode(b[12], b[13], b[14], b[15]) !== 'IHDR') return { error: 'the PNG is damaged or cut short' };
    const be32 = i => b[i] * 16777216 + (b[i + 1] << 16) + (b[i + 2] << 8) + b[i + 3];
    type = 'png'; width = be32(16); height = be32(20);
  } else {
    return { error: 'not a JPEG or PNG file' };
  }
  if (!width || !height) return { error: 'the image has no pixels' };
  if (width > 466 || height > 466) return { error: 'the image is ' + width + ' x ' + height + ': at most 466 x 466 pixels, the size of the screen' };
  return { type, width, height };
}

// The images a config uses, each once: face backgrounds and image widgets.
function configImages(cfg) {
  const out = [];
  const add = n => { if (typeof n === 'string' && n && !out.includes(n)) out.push(n); };
  for (const f of Array.isArray(cfg && cfg.faces) ? cfg.faces : []) {
    add(f && f.bgImage);
    for (const w of Array.isArray(f && f.widgets) ? f.widgets : []) if (w && w.type === 'image') add(w.image);
  }
  return out;
}

// ------------------------------------------------------------------- auth

async function auth(request, env, path) {
  if (path === '/api/me') {
    const user = await sessionUser(request, env);
    return json({ signedIn: !!user, email: user ? user.email : null, name: user ? user.name : null, picture: user ? user.picture : null, clientId: env.GOOGLE_CLIENT_ID || '' });
  }
  if (request.method !== 'POST') return json({ ok: false, error: 'POST only' }, 405);
  if (path === '/api/auth/logout') {
    return json({ ok: true }, 200, { 'set-cookie': `${COOKIE}=; Path=/; Max-Age=0; HttpOnly; Secure; SameSite=Lax` });
  }
  if (path === '/api/auth/google') {
    if (!env.GOOGLE_CLIENT_ID) return json({ ok: false, error: 'sign-in is not set up on this site yet' }, 503);
    if (!env.SESSION_SECRET) return json({ ok: false, error: 'the site has no session secret' }, 503);
    let body;
    try { body = await request.json(); } catch (e) { return json({ ok: false, error: 'bad request' }, 400); }
    let claims;
    try { claims = await verifyGoogleIdToken(body.credential || '', env.GOOGLE_CLIENT_ID); }
    catch (e) { return json({ ok: false, error: 'Google sign-in was not accepted: ' + e.message }, 401); }
    const session = { sub: claims.sub, email: claims.email, name: claims.name || '', picture: claims.picture || '', exp: Math.floor(Date.now() / 1000) + SESSION_SECONDS };
    const payload = b64url(utf8(JSON.stringify(session)));
    const sig = await hmac(env.SESSION_SECRET, payload);
    return json({ ok: true, email: session.email, name: session.name }, 200,
      { 'set-cookie': `${COOKIE}=${payload}.${sig}; Path=/; Max-Age=${SESSION_SECONDS}; HttpOnly; Secure; SameSite=Lax` });
  }
  return json({ ok: false, error: 'not found' }, 404);
}

async function sessionUser(request, env) {
  const raw = cookies(request)[COOKIE];
  if (!raw || !env.SESSION_SECRET) return null;
  const dot = raw.lastIndexOf('.');
  if (dot < 0) return null;
  const payload = raw.slice(0, dot), sig = raw.slice(dot + 1);
  if (!sameString(await hmac(env.SESSION_SECRET, payload), sig)) return null;
  try {
    const session = JSON.parse(fromUtf8(b64urlDecode(payload)));
    if (!session.sub || !(session.exp > Date.now() / 1000)) return null;
    return session;
  } catch (e) { return null; }
}

// Verifies an ID token from Google Identity Services: RS256 signature against
// Google's published keys, issuer, audience (our client id), expiry, and that
// Google has verified the email.
async function verifyGoogleIdToken(token, clientId) {
  const parts = token.split('.');
  if (parts.length !== 3) throw new Error('malformed token');
  let header, claims;
  try { header = JSON.parse(fromUtf8(b64urlDecode(parts[0]))); claims = JSON.parse(fromUtf8(b64urlDecode(parts[1]))); }
  catch (e) { throw new Error('malformed token'); }
  if (header.alg !== 'RS256') throw new Error('unexpected algorithm');
  const jwks = await (await fetch('https://www.googleapis.com/oauth2/v3/certs', { cf: { cacheTtl: 3600, cacheEverything: true } })).json();
  const jwk = (jwks.keys || []).find(k => k.kid === header.kid);
  if (!jwk) throw new Error('unknown signing key');
  const key = await crypto.subtle.importKey('jwk', jwk, { name: 'RSASSA-PKCS1-v1_5', hash: 'SHA-256' }, false, ['verify']);
  const ok = await crypto.subtle.verify('RSASSA-PKCS1-v1_5', key, b64urlDecode(parts[2]), utf8(parts[0] + '.' + parts[1]));
  if (!ok) throw new Error('bad signature');
  if (claims.iss !== 'accounts.google.com' && claims.iss !== 'https://accounts.google.com') throw new Error('wrong issuer');
  if (claims.aud !== clientId) throw new Error('token is for another site');
  if (!(claims.exp > Date.now() / 1000)) throw new Error('token has expired');
  if (!claims.email_verified) throw new Error('email not verified');
  return claims;
}

// ------------------------------------------------------------ user storage
// KV, with an index per user kept by hand so a save shows up at once (KV's own
// listing can lag by up to a minute).

const key = (user, ...rest) => ['u', user.sub, ...rest].join(':');

async function readIndex(env, user) {
  const idx = await env.HG.get(key(user, 'index'), 'json');
  return Object.assign({ configs: {}, recordings: {}, images: {}, virtual: [], assignments: {}, nextId: 1 }, idx || {});
}
const writeIndex = (env, user, idx) => env.HG.put(key(user, 'index'), JSON.stringify(idx));

const isSynthetic = mac => /^02:00:00:00:00:/.test(mac || '');
const syntheticMac = id => '02:00:00:00:00:' + id.toString(16).toUpperCase().padStart(2, '0');

// An alert rule as the page sends it: known keys of the right type only.
const ALERT_KEYS = { name: 'string', enabled: 'boolean', channel: 'string', unit: 'string', when: 'string', value: 'number', low: 'number', high: 'number',
  for: 'number', hold: 'number', message: 'string', color: 'string', face: 'number', restore: 'boolean', gauge: 'string' };
const cleanAlert = r => Object.fromEntries(Object.entries(r && typeof r === 'object' ? r : {})
  .filter(([k, v]) => ALERT_KEYS[k] === typeof v && (typeof v !== 'string' || v.length <= 64)));

async function userApi(request, env, path, user) {
  const seg = path.split('/').filter(Boolean);
  const area = seg[1];
  if (!['configs', 'assignments', 'virtual', 'recordings', 'settings', 'images'].includes(area)) return null;
  if (!user) return json({ ok: false, error: 'sign in to keep work in your account' }, 401);
  if (!env.HG) return json({ ok: false, error: 'storage is not set up on this site' }, 503);
  const method = request.method;
  const name = seg[2];
  const idx = await readIndex(env, user);

  // ---- configs
  if (area === 'configs') {
    if (!name) {
      if (method !== 'GET') return json({ ok: false, error: 'method' }, 405);
      return json(Object.entries(idx.configs).map(([n, m]) => ({ name: n, ...m })));
    }
    if (!NAME_RE.test(name)) return json({ ok: false, error: 'bad config name' }, 400);
    if (method === 'GET') {
      const text = await env.HG.get(key(user, 'cfg', name));
      if (text == null) return json({ ok: false, error: 'no such config' }, 404);
      return new Response(text, { headers: { 'content-type': 'application/json; charset=utf-8', 'cache-control': 'no-store' } });
    }
    if (method === 'PUT') {
      const text = await request.text();
      if (text.length > MAX_CONFIG_BYTES) return json({ ok: false, error: 'config is larger than the 64 KB limit' }, 400);
      let cfg;
      try { cfg = JSON.parse(text); } catch (e) { return json({ ok: false, error: 'not valid JSON: ' + e.message }, 400); }
      if (!cfg || typeof cfg !== 'object' || !Array.isArray(cfg.faces) || !cfg.faces.length) return json({ ok: false, error: "'faces' must list at least one face" }, 400);
      if (!idx.configs[name] && Object.keys(idx.configs).length >= MAX_CONFIGS) return json({ ok: false, error: 'at most ' + MAX_CONFIGS + ' configs per account' }, 409);
      await env.HG.put(key(user, 'cfg', name), text);
      if ('images' in cfg) return json({ ok: false, error: "'images' is filled in by the hub on the way to a gauge: leave it out" }, 400);
      idx.configs[name] = { size: text.length, crc: crc32(text), title: typeof cfg.name === 'string' ? cfg.name : '', faces: cfg.faces.length, images: configImages(cfg) };
      await writeIndex(env, user, idx);
      return json({ ok: true });
    }
    if (method === 'DELETE') {
      if (!idx.configs[name]) return json({ ok: false, error: 'no such config' }, 404);
      await env.HG.delete(key(user, 'cfg', name));
      delete idx.configs[name];
      for (const mac of Object.keys(idx.assignments)) if (idx.assignments[mac] === name) delete idx.assignments[mac];
      await writeIndex(env, user, idx);
      return json({ ok: true });
    }
    return json({ ok: false, error: 'method' }, 405);
  }

  // ---- assignments
  if (area === 'assignments') {
    if (!name) return json(idx.assignments);
    if (method !== 'PUT') return json({ ok: false, error: 'method' }, 405);
    const mac = name.toUpperCase();
    if (!MAC_RE.test(mac)) return json({ ok: false, error: 'MAC must look like AA:BB:CC:DD:EE:FF' }, 400);
    let body;
    try { body = await request.json(); } catch (e) { body = {}; }
    const cfgName = body.name || '';
    if (cfgName && !idx.configs[cfgName]) return json({ ok: false, error: 'no such config' }, 404);
    if (cfgName) idx.assignments[mac] = cfgName; else delete idx.assignments[mac];
    await writeIndex(env, user, idx);
    return json({ ok: true });
  }

  // ---- planned gauges, with the hub's rules
  if (area === 'virtual') {
    let body = {};
    if (method === 'POST' || method === 'PUT') { try { body = await request.json(); } catch (e) { body = {}; } }
    if (!name) {
      if (method === 'GET') return json(idx.virtual);
      if (method !== 'POST') return json({ ok: false, error: 'method' }, 405);
      if (idx.virtual.length >= MAX_PLANNED) return json({ ok: false, error: 'no room: at most ' + MAX_PLANNED + ' planned gauges' }, 409);
      const id = idx.nextId++;
      const entry = { id, label: String(body.label || 'Gauge ' + id).slice(0, 24), mac: syntheticMac(id), face: 0 };
      if (body.mac) {
        const mac = String(body.mac).toUpperCase();
        if (!MAC_RE.test(mac) || idx.virtual.some(v => v.mac === mac)) return json({ ok: false, error: 'MAC must look like AA:BB:CC:DD:EE:FF and not already be linked' }, 400);
        entry.mac = mac;
      }
      idx.virtual.push(entry);
      await writeIndex(env, user, idx);
      return json({ ok: true, id });
    }
    const id = parseInt(name, 10);
    const entry = idx.virtual.find(v => v.id === id);
    if (!entry) return json({ ok: false, error: 'no such planned gauge' }, 404);
    if (method === 'DELETE') {
      idx.virtual = idx.virtual.filter(v => v !== entry);
      if (isSynthetic(entry.mac)) delete idx.assignments[entry.mac];
      await writeIndex(env, user, idx);
      return json({ ok: true });
    }
    if (method !== 'PUT') return json({ ok: false, error: 'method' }, 405);
    if (typeof body.label === 'string') entry.label = body.label.slice(0, 24);
    if (Number.isInteger(body.face)) entry.face = Math.max(0, Math.min(7, body.face));
    if (typeof body.mac === 'string') {
      const mac = body.mac ? body.mac.toUpperCase() : syntheticMac(entry.id);
      if (!MAC_RE.test(mac) || idx.virtual.some(v => v !== entry && v.mac === mac)) return json({ ok: false, error: 'MAC must look like AA:BB:CC:DD:EE:FF and not already be linked' }, 400);
      if (mac !== entry.mac) {
        // The assignment moves with the gauge unless the new MAC already has one.
        if (idx.assignments[entry.mac] && !idx.assignments[mac]) idx.assignments[mac] = idx.assignments[entry.mac];
        delete idx.assignments[entry.mac];
        entry.mac = mac;
      }
    }
    await writeIndex(env, user, idx);
    return json({ ok: true });
  }

  // ---- settings: the alerts, and the simulator's speed and overrides, follow the account
  if (area === 'settings' && name === 'themecolours') {
    if (method === 'GET') return json(idx.settings && idx.settings.themeColours ? idx.settings.themeColours : null);
    if (method !== 'PUT') return json({ ok: false, error: 'method' }, 405);
    let body;
    try { body = await request.json(); } catch (e) { return json({ ok: false, error: 'bad request' }, 400); }
    const hex = /^#([0-9a-fA-F]{3}|[0-9a-fA-F]{6})$/;
    if (!Array.isArray(body) || body.length < 8 || body.length > 32 || !body.every(c => c && hex.test(c.value) && String(c.name || '').length <= 16)) {
      return json({ ok: false, error: 'theme colours must be a list of 8 to 32, each {value: "#RRGGBB", name}' }, 400);
    }
    idx.settings = { ...(idx.settings || {}), themeColours: body.map(c => ({ value: c.value, name: String(c.name || '') })) };
    await writeIndex(env, user, idx);
    return json({ ok: true });
  }
  if (area === 'settings' && name === 'alerts') {
    if (method === 'GET') return json(idx.settings && idx.settings.alerts ? idx.settings.alerts : null);
    if (method !== 'PUT') return json({ ok: false, error: 'method' }, 405);
    let body;
    try { body = await request.json(); } catch (e) { return json({ ok: false, error: 'bad request' }, 400); }
    if (!Array.isArray(body) || body.length > 16) return json({ ok: false, error: 'alerts must be a list of at most 16' }, 400);
    idx.settings = { ...(idx.settings || {}), alerts: body.map(cleanAlert) };
    await writeIndex(env, user, idx);
    return json({ ok: true });
  }
  if (area === 'settings') {
    if (name !== 'sim') return json({ ok: false, error: 'not found' }, 404);
    if (method === 'GET') return json(idx.settings && idx.settings.sim ? idx.settings.sim : null);
    if (method !== 'PUT') return json({ ok: false, error: 'method' }, 405);
    let body;
    try { body = await request.json(); } catch (e) { return json({ ok: false, error: 'bad request' }, 400); }
    const speed = Math.max(0.05, Math.min(10, +body.speed || 1));
    const overrides = (Array.isArray(body.overrides) ? body.overrides : []).slice(0, 16)
      .filter(o => o && typeof o.channel === 'string' && o.channel.length <= 48)
      .map(o => o.hold != null ? { channel: o.channel, hold: +o.hold } : { channel: o.channel, min: +o.min, max: +o.max });
    idx.settings = { ...(idx.settings || {}), sim: { speed, overrides } };
    await writeIndex(env, user, idx);
    return json({ ok: true });
  }

  // ---- images: the file itself, checked as the hub checks it
  if (area === 'images') {
    if (!name) {
      if (method !== 'GET') return json({ ok: false, error: 'method' }, 405);
      const list = Object.entries(idx.images).map(([n, m]) => ({ name: n, ...m }));
      const used = list.reduce((s, m) => s + m.size, 0);
      return json({ images: list, free: Math.max(0, ACCOUNT_IMAGE_BYTES - used), total: ACCOUNT_IMAGE_BYTES, max_bytes: MAX_IMAGE_BYTES, max_count: MAX_IMAGES });
    }
    if (!NAME_RE.test(name)) return json({ ok: false, error: 'bad image name' }, 400);
    const meta = idx.images[name];
    if (method === 'GET') {
      const data = meta ? await env.HG.get(key(user, 'img', name), 'arrayBuffer') : null;
      if (!data) return json({ ok: false, error: 'no such image' }, 404);
      return new Response(data, { headers: { 'content-type': meta.type === 'png' ? 'image/png' : 'image/jpeg', 'cache-control': 'no-store' } });
    }
    if (method === 'PUT') {
      const bytes = new Uint8Array(await request.arrayBuffer());
      if (!bytes.length || bytes.length > MAX_IMAGE_BYTES) return json({ ok: false, error: 'an image can be at most 256 KB' }, 400);
      const info = probeImage(bytes);
      if (info.error) return json({ ok: false, error: info.error }, 400);
      if (!meta && Object.keys(idx.images).length >= MAX_IMAGES) return json({ ok: false, error: 'at most ' + MAX_IMAGES + ' images per account; delete one first' }, 409);
      const used = Object.values(idx.images).reduce((s, m) => s + m.size, 0) - (meta ? meta.size : 0);
      if (used + bytes.length > ACCOUNT_IMAGE_BYTES) return json({ ok: false, error: 'no room: images in an account come to at most 4 MB' }, 409);
      await env.HG.put(key(user, 'img', name), bytes);
      idx.images[name] = { size: bytes.length, crc: crc32Bytes(bytes), width: info.width, height: info.height, type: info.type };
      await writeIndex(env, user, idx);
      return json({ ok: true });
    }
    if (method === 'DELETE') {
      if (!meta) return json({ ok: false, error: 'no such image' }, 404);
      await env.HG.delete(key(user, 'img', name));
      delete idx.images[name];
      await writeIndex(env, user, idx);
      return json({ ok: true });
    }
    return json({ ok: false, error: 'method' }, 405);
  }

  // ---- recordings
  if (area === 'recordings') {
    if (!name) {
      if (method !== 'GET') return json({ ok: false, error: 'method' }, 405);
      return json(Object.entries(idx.recordings).map(([n, m]) => ({ name: n, ...m, recording: false })));
    }
    if (!NAME_RE.test(name)) return json({ ok: false, error: 'bad recording name' }, 400);
    if (method === 'GET') {
      const text = await env.HG.get(key(user, 'rec', name));
      if (text == null) return json({ ok: false, error: 'no such recording' }, 404);
      return new Response(text, { headers: { 'content-type': 'application/json; charset=utf-8', 'cache-control': 'no-store' } });
    }
    if (method === 'PUT') {
      const text = await request.text();
      if (text.length > MAX_RECORDING_BYTES) return json({ ok: false, error: 'recording is larger than the 1 MB limit' }, 400);
      if (!text.startsWith('{"kind":"haltech-gauges-recording"')) return json({ ok: false, error: 'that file is not a recording' }, 400);
      let rec;
      try { rec = JSON.parse(text); } catch (e) { return json({ ok: false, error: 'not valid JSON: ' + e.message }, 400); }
      if (!Array.isArray(rec.channels) || !Array.isArray(rec.rows)) return json({ ok: false, error: 'the recording has no rows' }, 400);
      if (!idx.recordings[name] && Object.keys(idx.recordings).length >= MAX_RECORDINGS) return json({ ok: false, error: 'at most ' + MAX_RECORDINGS + ' recordings per account' }, 409);
      await env.HG.put(key(user, 'rec', name), text);
      idx.recordings[name] = { bytes: text.length, duration_ms: rec.duration_ms || 0 };
      await writeIndex(env, user, idx);
      return json({ ok: true });
    }
    if (method === 'DELETE') {
      if (!idx.recordings[name]) return json({ ok: false, error: 'no such recording' }, 404);
      await env.HG.delete(key(user, 'rec', name));
      delete idx.recordings[name];
      await writeIndex(env, user, idx);
      return json({ ok: true });
    }
    return json({ ok: false, error: 'method' }, 405);
  }
  return null;
}
