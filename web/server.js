const fs = require('node:fs');
const { pipeline } = require('node:stream/promises');
const path = require('node:path');
const { randomUUID } = require('node:crypto');
const { spawn, execFileSync } = require('node:child_process');
const Fastify = require('fastify');
const multipart = require('@fastify/multipart');

const projectRoot = path.resolve(__dirname, '..');
const host = process.env.CAD_WEB_HOST || '127.0.0.1';
const port = Number.parseInt(process.env.CAD_WEB_PORT || '3307', 10);
const analyzerExe = process.env.CAD_ANALYZER_EXE || path.join(projectRoot, 'build', 'x64-release', 'cad_model_analyzer.exe');
const analyzerScript = process.env.CAD_ANALYZER_SCRIPT || path.join(projectRoot, 'run-analyzer.bat');
const uploadDir = process.env.CAD_UPLOAD_DIR || path.join(projectRoot, 'output', 'web-uploads');
const reportDir = process.env.CAD_REPORT_DIR || path.join(projectRoot, 'output', 'web-reports');
const maxUploadBytes = Number.parseInt(process.env.CAD_MAX_UPLOAD_MB || '50', 10) * 1024 * 1024;

function ensureRuntimeDirs() {
  fs.mkdirSync(uploadDir, { recursive: true });
  fs.mkdirSync(reportDir, { recursive: true });
}

function safeOriginalName(filename) {
  return path.basename(filename || 'model.step').replace(/[^a-zA-Z0-9._-]/g, '_');
}

function hasStepExtension(filename) {
  return /\.(step|stp)$/i.test(filename || '');
}

function toWindowsPath(filePath) {
  if (process.platform !== 'linux') return filePath;
  if (!filePath.startsWith('/mnt/')) return filePath;
  try {
    return execFileSync('wslpath', ['-w', filePath], { encoding: 'utf8' }).trim();
  } catch (_) {
    return filePath;
  }
}

function runAnalyzer(inputPath, reportPath) {
  return new Promise((resolve, reject) => {
    const isWindowsExeFromWsl = process.platform === 'linux' && /\.exe$/i.test(analyzerExe);
    const useWindowsScript = process.platform === 'win32' && fs.existsSync(analyzerScript);
    const command = isWindowsExeFromWsl && fs.existsSync(analyzerScript)
      ? 'cmd.exe'
      : useWindowsScript
        ? analyzerScript
        : analyzerExe;
    const analyzerArgs = isWindowsExeFromWsl
      ? [toWindowsPath(inputPath), '-o', toWindowsPath(reportPath)]
      : [inputPath, '-o', reportPath];
    const args = command === 'cmd.exe'
      ? ['/c', toWindowsPath(analyzerScript), ...analyzerArgs]
      : analyzerArgs;
    const child = spawn(command, args, {
      cwd: projectRoot,
      windowsHide: true,
      env: process.env,
    });

    let stdout = '';
    let stderr = '';
    child.stdout.on('data', (chunk) => { stdout += chunk.toString(); });
    child.stderr.on('data', (chunk) => { stderr += chunk.toString(); });
    child.on('error', reject);
    child.on('close', (code) => {
      if (code !== 0) {
        const message = stderr.trim() || stdout.trim() || `cad_model_analyzer exited with ${code}`;
        reject(new Error(message));
        return;
      }
      resolve({ stdout, stderr });
    });
  });
}

function createApp() {
  const app = Fastify({ logger: false });

  app.register(multipart, {
    limits: {
      fileSize: maxUploadBytes,
      files: 1,
    },
  });

  app.get('/api/health', async () => ({
    ok: true,
    analyzer_exists: fs.existsSync(analyzerExe),
    analyzer: analyzerExe,
  }));

  app.post('/api/analyze', async (request, reply) => {
    ensureRuntimeDirs();
    const part = await request.file();
    if (!part) {
      return reply.code(400).send({ error: 'missing file field' });
    }

    const originalName = safeOriginalName(part.filename);
    if (!hasStepExtension(originalName)) {
      await part.file.resume();
      return reply.code(400).send({ error: 'only .step/.stp files are supported' });
    }

    if (!fs.existsSync(analyzerExe)) {
      await part.file.resume();
      return reply.code(500).send({ error: `analyzer executable not found: ${analyzerExe}` });
    }

    const jobId = randomUUID();
    const inputPath = path.join(uploadDir, `${jobId}-${originalName}`);
    const reportPath = path.join(reportDir, `${jobId}.json`);

    await pipeline(part.file, fs.createWriteStream(inputPath));

    try {
      await runAnalyzer(inputPath, reportPath);
      const report = JSON.parse(await fs.promises.readFile(reportPath, 'utf8'));
      return {
        job_id: jobId,
        filename: originalName,
        report,
      };
    } catch (error) {
      return reply.code(500).send({
        error: error instanceof Error ? error.message : String(error),
        job_id: jobId,
      });
    }
  });

  app.get('/', async (_, reply) => reply.type('text/html; charset=utf-8').send(fs.createReadStream(path.join(__dirname, 'public', 'index.html'))));
  app.get('/app.js', async (_, reply) => reply.type('application/javascript; charset=utf-8').send(fs.createReadStream(path.join(__dirname, 'public', 'app.js'))));
  app.get('/style.css', async (_, reply) => reply.type('text/css; charset=utf-8').send(fs.createReadStream(path.join(__dirname, 'public', 'style.css'))));

  return app;
}

async function main() {
  ensureRuntimeDirs();
  const app = createApp();
  await app.listen({ host, port });
  console.log(`CAD Model Analyzer web UI listening on http://${host}:${port}`);
}

if (require.main === module) {
  main().catch((error) => {
    console.error(error);
    process.exit(1);
  });
}

module.exports = { createApp, runAnalyzer, hasStepExtension };
