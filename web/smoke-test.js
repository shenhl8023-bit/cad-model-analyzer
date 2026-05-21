const path = require('node:path');
const fs = require('node:fs');
const { createApp } = require('./server');

const projectRoot = path.resolve(__dirname, '..');

async function main() {
  const app = createApp();
  await app.ready();

  const health = await app.inject({ method: 'GET', url: '/api/health' });
  if (health.statusCode !== 200) {
    throw new Error(`health check failed: ${health.statusCode}`);
  }

  const page = await app.inject({ method: 'GET', url: '/' });
  if (page.statusCode !== 200 || !page.body.includes('CAD Model Analyzer')) {
    throw new Error('index page smoke check failed');
  }

  const sample = process.env.CAD_SMOKE_STEP || path.join(projectRoot, 'output', 'batch-input', 'screw.step');
  const requiredAnalyzer = process.env.CAD_ANALYZER_EXE || path.join(projectRoot, 'build', 'x64-release', 'cad_model_analyzer.exe');
  if (!fs.existsSync(sample)) {
    throw new Error(`sample STEP file not found: ${sample}`);
  }
  if (!fs.existsSync(requiredAnalyzer)) {
    throw new Error(`analyzer executable not found: ${requiredAnalyzer}`);
  }

  {
    const form = new FormData();
    const bytes = await fs.promises.readFile(sample);
    form.append('file', new Blob([bytes]), path.basename(sample));
    const res = await app.inject({
      method: 'POST',
      url: '/api/analyze',
      payload: form,
      headers: form.headers || {},
    });
    if (res.statusCode !== 200) {
      throw new Error(`analyze smoke check failed: ${res.statusCode} ${res.body}`);
    }
    const payload = JSON.parse(res.body);
    if (payload.report?.quality?.status !== 'ok' || payload.report?.topology?.solid !== 1) {
      throw new Error('unexpected analyze smoke result');
    }
  }

  await app.close();
  console.log('web smoke test OK');
}

main().catch((error) => {
  console.error(error);
  process.exit(1);
});
