// Assembles ./dist for the Cloudflare Worker: the page flagged as the online copy,
// the templates with a list of them, the fonts and the channel list. No
// dependencies, so Cloudflare's build can run it with plain Node.
import { copyFileSync, mkdirSync, readdirSync, readFileSync, rmSync, writeFileSync } from 'node:fs';
import { join } from 'node:path';
import { fileURLToPath } from 'node:url';

const root = fileURLToPath(new URL('..', import.meta.url));
const dist = join(root, 'dist');
// Empty dist rather than removing it: a running wrangler dev holds the directory open on Windows.
mkdirSync(dist, { recursive: true });
for (const entry of readdirSync(dist)) rmSync(join(dist, entry), { recursive: true, force: true });
mkdirSync(join(dist, 'data', 'templates'), { recursive: true });
mkdirSync(join(dist, 'fonts'), { recursive: true });

// The page, marked so it starts in demo mode without probing for a hub.
const page = readFileSync(join(root, 'web', 'index.html'), 'utf8');
if (!page.includes('<html lang="en">')) throw new Error('web/index.html: expected <html lang="en">');
writeFileSync(join(dist, 'index.html'), page.replace('<html lang="en">', '<html lang="en" data-online="1">'));

// Templates, and the list the hub's /api/templates returns.
const templateDir = join(root, 'assets', 'templates');
const list = [];
for (const file of readdirSync(templateDir).filter(f => f.endsWith('.json')).sort()) {
  const name = file.slice(0, -5);
  const cfg = JSON.parse(readFileSync(join(templateDir, file), 'utf8'));
  copyFileSync(join(templateDir, file), join(dist, 'data', 'templates', file));
  list.push({ name, title: cfg.name || name });
}
writeFileSync(join(dist, 'data', 'templates.json'), JSON.stringify(list));

// Base recordings and their list, from the first line of each file.
const recDir = join(root, 'assets', 'recordings');
mkdirSync(join(dist, 'data', 'recordings'), { recursive: true });
const recs = [];
for (const file of readdirSync(recDir).filter(f => f.endsWith('.json')).sort()) {
  const text = readFileSync(join(recDir, file), 'utf8');
  const name = file.slice(0, -5);
  let head = {};
  try { head = JSON.parse(text.split('\n', 1)[0] + ']}'); } catch (e) { throw new Error(file + ': not a recording (bad header line)'); }
  const tail = text.slice(-64).match(/"duration_ms":(\d+)/);
  copyFileSync(join(recDir, file), join(dist, 'data', 'recordings', file));
  recs.push({ name, title: head.name || name, description: head.description || '', duration_ms: tail ? +tail[1] : 0, bytes: text.length });
}
writeFileSync(join(dist, 'data', 'recordings.json'), JSON.stringify(recs));

// Fonts and their manifest.
const fontDir = join(root, 'assets', 'fonts');
for (const file of readdirSync(fontDir).filter(f => f.endsWith('.ttf'))) {
  copyFileSync(join(fontDir, file), join(dist, 'fonts', file));
}
copyFileSync(join(fontDir, 'fonts.json'), join(dist, 'data', 'fonts.json'));

// The channel list, generated from the firmware tables by scripts/gen_channels.ps1.
const channels = join(root, 'web', 'data', 'channels.json');
try {
  copyFileSync(channels, join(dist, 'data', 'channels.json'));
} catch (e) {
  throw new Error('web/data/channels.json is missing: run scripts/gen_channels.ps1 first');
}

console.log(`dist: page ${page.length} bytes, ${list.length} templates, ${readdirSync(join(dist, 'fonts')).length} fonts, ${recs.length} library recordings`);
