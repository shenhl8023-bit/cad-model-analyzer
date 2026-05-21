const $ = (id) => document.getElementById(id);
let lastReportText = '';

function fmtNumber(value, digits = 3) {
  if (typeof value !== 'number' || Number.isNaN(value)) return '-';
  return Number.isInteger(value) ? String(value) : value.toFixed(digits);
}

function setMessage(text, kind = '') {
  $('message').textContent = text;
  $('message').className = `message ${kind}`.trim();
}

async function checkHealth() {
  try {
    const res = await fetch('/api/health');
    const data = await res.json();
    $('health').textContent = data.analyzer_exists ? '服务正常，分析器可用' : '服务正常，但未找到分析器 exe';
    $('health').className = data.analyzer_exists ? 'health ok' : 'health warn';
  } catch (error) {
    $('health').textContent = '服务不可用';
    $('health').className = 'health error';
  }
}

function renderReport(report) {
  const quality = report.quality || {};
  const topology = report.topology || {};
  const metrics = report.metrics || {};

  $('summary').classList.remove('hidden');
  $('details').classList.remove('hidden');
  $('jsonCard').classList.remove('hidden');

  $('qualityStatus').textContent = quality.status || '-';
  $('complexity').textContent = quality.complexity_level || '-';
  $('issueCount').textContent = quality.issue_count ?? '-';
  $('analysisTime').textContent = `${report.metadata?.analysis_time_ms ?? '-'} ms`;
  $('solidCount').textContent = topology.solid ?? '-';
  $('faceCount').textContent = topology.face ?? '-';
  $('edgeCount').textContent = topology.edge ?? '-';
  $('volume').textContent = fmtNumber(metrics.volume);

  const checks = [
    ['闭合实体候选', quality.closed_solid_candidate],
    ['存在自由边', quality.has_free_edges],
    ['存在非流形边', quality.has_non_manifold_edges],
    ['体积为正', quality.has_positive_volume],
    ['多 Solid', quality.multi_solid],
    ['Shell-only', quality.shell_only],
  ];
  $('checks').innerHTML = checks.map(([label, value]) => {
    const className = value ? 'true' : 'false';
    return `<li><span>${label}</span><strong class="${className}">${value === true ? '是' : value === false ? '否' : '-'}</strong></li>`;
  }).join('');

  lastReportText = JSON.stringify(report, null, 2);
  $('json').textContent = lastReportText;
}

async function analyze() {
  const file = $('file').files[0];
  if (!file) {
    setMessage('请先选择 STEP/STP 文件。', 'warn');
    return;
  }
  if (!/\.(step|stp)$/i.test(file.name)) {
    setMessage('只支持 .step / .stp 文件。', 'warn');
    return;
  }

  const data = new FormData();
  data.append('file', file);

  $('analyze').disabled = true;
  setMessage('正在上传并分析...', '');
  try {
    const res = await fetch('/api/analyze', { method: 'POST', body: data });
    const payload = await res.json();
    if (!res.ok) {
      throw new Error(payload.error || '分析失败');
    }
    renderReport(payload.report);
    setMessage(`分析完成：${payload.filename}`, 'ok');
  } catch (error) {
    setMessage(error.message || String(error), 'error');
  } finally {
    $('analyze').disabled = false;
  }
}

$('analyze').addEventListener('click', analyze);
$('copy').addEventListener('click', async () => {
  await navigator.clipboard.writeText(lastReportText);
  setMessage('JSON 已复制。', 'ok');
});
checkHealth();
