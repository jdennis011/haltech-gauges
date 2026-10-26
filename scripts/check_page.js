// Checks web/index.html's script blocks: a syntax pass with `node --check`,
// and no top-level function declared twice (the later one silently wins).
//   node scripts/check_page.js [path/to/index.html]
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import { execFileSync } from 'node:child_process';
import { fileURLToPath } from 'node:url';

const here = path.dirname(fileURLToPath(import.meta.url));
const file = process.argv[2] || path.join(here, '..', 'web', 'index.html');
const html = fs.readFileSync(file, 'utf8');
const scripts = [...html.matchAll(/<script>([\s\S]*?)<\/script>/g)].map(m => m[1]);
let bad = 0;
scripts.forEach((src, i) => {
  const tmp = path.join(os.tmpdir(), 'hg_page_' + i + '.cjs');
  fs.writeFileSync(tmp, src);
  try {
    execFileSync(process.execPath, ['--check', tmp], { stdio: 'inherit' });
  } catch (e) {
    bad++;
  }
  const seen = new Map();
  for (const m of src.matchAll(/^(?:async\s+)?function\s+(\w+)\s*\(/gm)) {
    const line = src.slice(0, m.index).split('\n').length;
    if (seen.has(m[1])) {
      console.error(file + ': function ' + m[1] + ' declared twice (script ' + i + ', lines ' + seen.get(m[1]) + ' and ' + line + ')');
      bad++;
    } else {
      seen.set(m[1], line);
    }
  }
});
console.log(bad ? 'page check FAILED' : 'page check ok (' + scripts.length + ' script block(s))');
process.exit(bad ? 1 : 0);
