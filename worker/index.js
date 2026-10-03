// The online copy of the hub's page, on Cloudflare. The page is the same file the
// hub serves; here it runs in demo mode (no hub, state kept in the browser). The
// data it needs, templates, the channel list and fonts, is served as static
// files behind the same /api paths the hub uses, so the page's code is identical.

const STATIC_API = {
  '/api/templates': '/data/templates.json',
  '/api/channels/meta': '/data/channels.json',
  '/api/fonts': '/data/fonts.json',
};

export default {
  async fetch(request, env) {
    const url = new URL(request.url);
    const path = url.pathname;

    let target = STATIC_API[path];
    if (!target && path.startsWith('/api/templates/')) {
      const name = path.slice('/api/templates/'.length);
      if (/^[A-Za-z0-9_-]{1,32}$/.test(name)) target = '/data/templates/' + name + '.json';
    }
    if (target) {
      const res = await env.ASSETS.fetch(new URL(target, url));
      if (!res.ok) return Response.json({ ok: false, error: 'not found' }, { status: 404 });
      return new Response(res.body, {
        status: 200,
        headers: { 'content-type': 'application/json; charset=utf-8', 'cache-control': 'public, max-age=300' },
      });
    }
    if (path.startsWith('/api/') || path === '/ws') {
      return Response.json({ ok: false, error: 'this is the online copy: there is no hub behind it' }, { status: 404 });
    }
    return env.ASSETS.fetch(request);
  },
};
