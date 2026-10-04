// The online copy of the hub's page, on Cloudflare. The page is the same file the
// hub serves; here it runs in demo mode (no hub). Three things live in this Worker:
//
// 1. Static data behind the same /api paths the hub uses (templates, the channel
//    list, fonts), so the page's code is identical.
// 2. Sign in with Google: the page posts Google's ID token, the Worker verifies it
//    against Google's keys and sets a signed session cookie.
// 3. Per-user storage in KV, behind the hub's own paths for configs, planned
//    gauges, assignments and recordings, so what you make follows your account
//    across devices. Signed out, the page keeps its work in the browser instead.

const STATIC_API = {
  '/api/templates': '/data/templates.json',
  '/api/channels/meta': '/data/channels.json',
  '/api/fonts': '/data/fonts.json',
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

export default {
  async fetch(request, env) {
    const url = new URL(request.url);
    const path = url.pathname;

    let target = STATIC_API[path];
    if (!target && path.startsWith('/api/templates/')) {
      const name = path.slice('/api/templates/'.length);
      if (NAME_RE.test(name)) target = '/data/templates/' + name + '.json';
    }
    if (target) {
      const res = await env.ASSETS.fetch(new URL(target, url));
      if (!res.ok) return json({ ok: false, error: 'not found' }, 404);
      return new Response(res.body, { status: 200, headers: { 'content-type': 'application/json; charset=utf-8', 'cache-control': 'public, max-age=300' } });
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
function crc32(text) {
  if (!crcTable) {
    crcTable = new Uint32Array(256);
    for (let n = 0; n < 256; n++) { let c = n; for (let k = 0; k < 8; k++) c = c & 1 ? 0xEDB88320 ^ (c >>> 1) : c >>> 1; crcTable[n] = c >>> 0; }
  }
  const bytes = utf8(text);
  let crc = 0xFFFFFFFF;
  for (const b of bytes) crc = crcTable[(crc ^ b) & 0xFF] ^ (crc >>> 8);
  return ((crc ^ 0xFFFFFFFF) >>> 0).toString(16).toUpperCase().padStart(8, '0');
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
  const header = JSON.parse(fromUtf8(b64urlDecode(parts[0])));
  const claims = JSON.parse(fromUtf8(b64urlDecode(parts[1])));
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
  return idx || { configs: {}, recordings: {}, virtual: [], assignments: {}, nextId: 1 };
}
const writeIndex = (env, user, idx) => env.HG.put(key(user, 'index'), JSON.stringify(idx));

const isSynthetic = mac => /^02:00:00:00:00:/.test(mac || '');
const syntheticMac = id => '02:00:00:00:00:' + id.toString(16).toUpperCase().padStart(2, '0');

async function userApi(request, env, path, user) {
  const seg = path.split('/').filter(Boolean);
  const area = seg[1];
  if (!['configs', 'assignments', 'virtual', 'recordings'].includes(area)) return null;
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
      idx.configs[name] = { size: text.length, crc: crc32(text), title: typeof cfg.name === 'string' ? cfg.name : '', faces: cfg.faces.length };
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
