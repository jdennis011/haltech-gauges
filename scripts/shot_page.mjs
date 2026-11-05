// Screenshots the hub page in headless Chrome, with no hub: opens web/index.html from disk
// (mock data), sets the viewport, optionally runs an expression on the page first, then
// writes a PNG. Needs Node 22 or newer (built-in WebSocket) and Chrome at its usual path.
//   node scripts/shot_page.mjs "file:///C:/.../web/index.html#configs-demo" 1400 1000 out.png
//   node scripts/shot_page.mjs <url> 420 900 phone.png "document.querySelector(\"#btn-help\").click()"
// Add --full for the whole page rather than the viewport. Page errors are printed.
import { spawn } from 'node:child_process';
import { writeFileSync, mkdtempSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join } from 'node:path';

const [url, w, h, out, expr] = process.argv.slice(2);
const full = process.argv.includes('--full');
const port = 9333 + Math.floor(Math.random() * 500);
const profile = mkdtempSync(join(tmpdir(), 'hg-shot-'));
const chrome = spawn('C:/Program Files/Google/Chrome/Application/chrome.exe',
  ['--headless=new', '--disable-gpu', '--hide-scrollbars', '--no-first-run', '--remote-debugging-port=' + port,
   '--user-data-dir=' + profile, '--allow-file-access-from-files', 'about:blank'], { stdio: 'ignore' });
const sleep = ms => new Promise(r => setTimeout(r, ms));
let version;
for (let i = 0; i < 50 && !version; i++) {
  try { version = await (await fetch(`http://127.0.0.1:${port}/json/version`)).json(); } catch { await sleep(200); }
}
if (!version) { chrome.kill(); throw new Error('chrome did not start'); }
const target = await (await fetch(`http://127.0.0.1:${port}/json/new?about:blank`, { method: 'PUT' })).json();
const ws = new WebSocket(target.webSocketDebuggerUrl);
await new Promise((res, rej) => { ws.onopen = res; ws.onerror = rej; });
let id = 0; const pending = new Map(); const events = [];
ws.onmessage = ev => {
  const m = JSON.parse(ev.data);
  if (m.id && pending.has(m.id)) { const { res, rej } = pending.get(m.id); pending.delete(m.id); m.error ? rej(new Error(JSON.stringify(m.error))) : res(m.result); }
  else if (m.method) events.push(m);
};
const send = (method, params = {}) => new Promise((res, rej) => { const i = ++id; pending.set(i, { res, rej }); ws.send(JSON.stringify({ id: i, method, params })); });
await send('Page.enable');
await send('Runtime.enable');
await send('Emulation.setDeviceMetricsOverride', { width: +w, height: +h, deviceScaleFactor: 1, mobile: +w < 800 });
await send('Page.navigate', { url });
for (let i = 0; i < 100 && !events.some(e => e.method === 'Page.loadEventFired'); i++) await sleep(100);
await sleep(1200);
if (expr) {
  const r = await send('Runtime.evaluate', { expression: expr, awaitPromise: true, returnByValue: true });
  console.log(JSON.stringify(r.result.value ?? r.result.description ?? r.result, null, 1));
  await sleep(600);
}
const errors = events.filter(e => e.method === 'Runtime.exceptionThrown').map(e => e.params.exceptionDetails.exception?.description || e.params.exceptionDetails.text);
if (errors.length) console.log('PAGE ERRORS:', errors);
const shot = await send('Page.captureScreenshot', { format: 'png', captureBeyondViewport: full, ...(full ? { clip: await (async () => { const m = await send('Page.getLayoutMetrics'); return { x: 0, y: 0, width: +w, height: Math.min(m.cssContentSize.height, 6000), scale: 1 }; })() } : {}) });
writeFileSync(out, Buffer.from(shot.data, 'base64'));
console.log('wrote', out);
ws.close(); chrome.kill();
