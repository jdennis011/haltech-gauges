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

console.log(`dist: page ${page.length} bytes, ${list.length} templates, ${readdirSync(join(dist, 'fonts')).length} fonts`);
