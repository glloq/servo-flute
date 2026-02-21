/***********************************************************************************************
 * web_content.h - Interface web embarquee en PROGMEM
 *
 * Single Page Application avec 3 onglets + overlay settings :
 * 1. Clavier virtuel (dynamique depuis cfg.notes[])
 * 2. Lecteur MIDI (upload + transport)
 * 3. Calibration (3 etapes : Doigts, Doigtes, Souffle)
 * + Overlay Settings (engrenage) : tous les parametres avances
 *
 * Interface responsive / mobile-friendly
 ***********************************************************************************************/
#ifndef WEB_CONTENT_H
#define WEB_CONTENT_H

const char WEB_HTML_CONTENT[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no">
<title>ServoFlute</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;
background:#1a1a2e;color:#e0e0e0;overflow-x:hidden;min-height:100vh;padding-bottom:36px}
.hdr{background:#16213e;padding:10px 16px;display:flex;justify-content:space-between;
align-items:center;border-bottom:2px solid #0f3460;position:sticky;top:0;z-index:10}
.hdr h1{font-size:1.1em;color:#e94560;display:flex;align-items:center}
.hdr-r{display:flex;align-items:center;gap:12px}
.dot{width:10px;height:10px;border-radius:50%;display:inline-block}
.dot.on{background:#4ecca3;box-shadow:0 0 6px #4ecca3}.dot.off{background:#777}
.unsaved-badge{display:none;background:#e9a645;color:#1a1a2e;font-size:.6em;padding:2px 6px;
border-radius:8px;font-weight:bold;margin-left:8px;vertical-align:middle}
.unsaved-badge.show{display:inline-block}
.gear-btn{background:none;border:none;color:#9aa;font-size:1.3em;cursor:pointer;padding:4px;
display:flex;align-items:center}
.gear-btn:hover{color:#e94560}
.tabs{display:flex;background:#16213e;border-bottom:1px solid #0f3460;overflow-x:auto}
.tabs button{flex:1;background:none;border:none;color:#9aa;padding:12px 8px;font-size:0.85em;
cursor:pointer;border-bottom:2px solid transparent;white-space:nowrap;min-width:80px;
display:flex;align-items:center;justify-content:center;gap:6px}
.tabs button.active{color:#e94560;border-bottom-color:#e94560}
.tabs button svg{width:14px;height:14px;flex-shrink:0}
.tab{display:none;padding:12px}.tab.active{display:block}
.section{background:#16213e;border-radius:8px;padding:12px;margin-bottom:12px;border:1px solid #0f3460}
.section h3{color:#e94560;font-size:0.9em;margin-bottom:8px}
.btn{padding:8px 16px;border:none;border-radius:6px;cursor:pointer;font-size:0.85em;
position:relative;transition:all .2s;display:inline-flex;align-items:center;gap:6px}
.btn-p{background:#e94560;color:#fff}.btn-p:hover{background:#d63650}
.btn-s{background:#0f3460;color:#e0e0e0;border:1px solid #1a4080}.btn-s:hover{background:#1a4080}
.btn-g{background:#4ecca3;color:#1a1a2e}.btn-g:hover{background:#3db892}
.btn:disabled{opacity:.4;cursor:default}
.btn-row{display:flex;gap:8px;margin-top:10px;flex-wrap:wrap}
.btn svg{width:14px;height:14px;flex-shrink:0}
.btn.loading{color:transparent !important;pointer-events:none}
.btn.loading::after{content:'';position:absolute;width:14px;height:14px;border:2px solid transparent;
border-top-color:#fff;border-radius:50%;animation:spin .6s linear infinite;
top:50%;left:50%;margin:-7px 0 0 -7px}
@keyframes spin{to{transform:rotate(360deg)}}
@keyframes testPulse{0%{box-shadow:0 0 0 0 rgba(78,204,163,.7)}70%{box-shadow:0 0 0 10px rgba(78,204,163,0)}100%{box-shadow:0 0 0 0 rgba(78,204,163,0)}}
.test-pulse{animation:testPulse .6s ease}
input[type=range]{width:100%;accent-color:#e94560}
input[type=number],input[type=text],input[type=password],select{background:#0d1b3e;border:1px solid #1a4080;
color:#e0e0e0;padding:6px 8px;border-radius:4px;font-size:0.85em;width:100%}
.cfg-row{display:flex;align-items:center;gap:8px;margin-bottom:6px}
.cfg-row label{flex:0 0 140px;font-size:0.8em;color:#aaa;text-align:right}
.cfg-row input,.cfg-row select{flex:1}
.keys{display:flex;flex-wrap:wrap;gap:6px;justify-content:center;padding:8px 0}
.key{background:linear-gradient(180deg,#2a2a4a,#1a1a2e);border:1px solid #0f3460;
border-radius:6px;padding:10px 8px;text-align:center;cursor:pointer;user-select:none;
min-width:60px;flex:0 0 auto;transition:all .15s}
.key.black{background:linear-gradient(180deg,#1a1a2e,#0a0a1e);border-color:#444}
.key.pressed,.key:active{background:#e94560;border-color:#e94560;transform:scale(.96)}
.note-name{display:block;font-weight:bold;font-size:1em;color:#fff}
.note-midi{display:block;font-size:0.65em;color:#9aa;margin-top:2px}
.key-shortcut{display:none;font-size:.55em;color:#777;margin-top:2px}
.key:hover .key-shortcut{display:block}
.kf-row{display:flex;gap:3px;justify-content:center;margin-top:4px}
.kf{width:8px;height:8px;border-radius:50%;border:1px solid #777}
.kf.c{background:#444}.kf.o{background:#4ecca3}
.flute-box{background:#0d1b3e;border-radius:8px;padding:12px;text-align:center;margin-bottom:8px}
.flute-box svg{width:100%;max-width:600px;height:auto}
.flute-hole{stroke:#5C4A0A;stroke-width:2;transition:all .2s}
.flute-hole.closed{fill:#3a2a0a}.flute-hole.open{fill:#4ecca3}
.flute-hole.thumb{filter:drop-shadow(0 0 3px #e94560)}
.flute-hole:hover{filter:drop-shadow(0 0 6px #e94560);cursor:pointer}
@keyframes holePulse{0%,100%{filter:drop-shadow(0 0 4px #4ecca3)}50%{filter:drop-shadow(0 0 14px #4ecca3)}}
.flute-hole.playing{animation:holePulse 1s ease-in-out infinite}
.flute-lbl{font-size:11px;fill:#9aa}
.flute-num{font-size:10px;fill:#fff;font-weight:bold;pointer-events:none}
.flute-info{text-align:center;font-size:0.8em;color:#9aa;margin-top:4px}
.toast-container{position:fixed;top:56px;right:12px;z-index:200;display:flex;flex-direction:column;
gap:8px;pointer-events:none}
.toast{padding:10px 16px;border-radius:8px;font-size:.82em;color:#fff;opacity:0;
transform:translateX(40px);transition:all .3s ease;display:flex;align-items:center;gap:8px;
max-width:320px;pointer-events:auto;box-shadow:0 4px 16px rgba(0,0,0,.4)}
.toast.show{opacity:1;transform:translateX(0)}
.toast.success{background:rgba(45,107,79,.95);border:1px solid #4ecca3}
.toast.error{background:rgba(107,45,58,.95);border:1px solid #e94560}
.toast.info{background:rgba(45,58,107,.95);border:1px solid #4a7eca}
.toast svg{flex-shrink:0;width:16px;height:16px}
.skeleton{background:linear-gradient(90deg,#16213e 25%,#1e2d50 50%,#16213e 75%);
background-size:200% 100%;animation:shimmer 1.5s infinite;border-radius:6px;min-height:20px}
@keyframes shimmer{0%{background-position:200% 0}100%{background-position:-200% 0}}
@keyframes fadeInUp{from{opacity:0;transform:translateY(12px)}to{opacity:1;transform:translateY(0)}}
.fade-in{animation:fadeInUp .3s ease forwards}
.fade-delay-1{animation-delay:.05s;opacity:0}.fade-delay-2{animation-delay:.1s;opacity:0}
.fade-delay-3{animation-delay:.15s;opacity:0}.fade-delay-4{animation-delay:.2s;opacity:0}
.steps{display:flex;align-items:center;justify-content:center;gap:0;padding:12px 0}
.step-dot{width:12px;height:12px;border-radius:50%;background:#444;cursor:pointer;transition:.2s}
.step-dot.active{background:#e94560;box-shadow:0 0 8px #e94560}.step-dot.done{background:#4ecca3}
.step-dot.locked{opacity:.4;cursor:not-allowed}
.step-line{width:40px;height:2px;background:#444}
.step-labels{display:flex;justify-content:center;gap:34px;font-size:0.75em;color:#9aa;margin-bottom:8px}
.cal-card{background:linear-gradient(135deg,#0d1b3e 0%,#101f45 100%);border:1px solid #1a4080;
border-radius:8px;padding:10px;margin-bottom:8px}
.cal-card h4{font-size:0.85em;color:#e94560;margin-bottom:6px}
.cal-card.pca-conflict{border-color:#e9a645;box-shadow:0 0 8px rgba(233,166,69,.3)}
.pca-warn{color:#e9a645;font-size:.75em;margin-top:4px;display:none}
.pca-conflict .pca-warn{display:block}
.fg-row{display:flex;align-items:center;gap:8px;padding:6px 4px;border-bottom:1px solid #0f3460}
.fg-row:last-child{border-bottom:none}
.fg-note{font-weight:bold;min-width:50px;font-size:0.9em}
.fg-midi{color:#9aa;font-size:0.75em;min-width:36px}
.fg-octave{background:#0f3460;padding:4px 10px;font-size:.75em;color:#e94560;
font-weight:bold;border-radius:4px;margin:6px 0}
.fg-dots{display:flex;gap:4px;flex:1}
.fg-dot{width:18px;height:18px;border-radius:50%;border:2px solid #777;cursor:pointer;transition:.15s}
.fg-dot.closed{background:#444}.fg-dot.open{background:#4ecca3;border-color:#4ecca3}
.fg-dot.thumb{border-style:dashed;border-color:#e94560}
.air-card{display:flex;align-items:center;gap:8px;padding:6px 0;border-bottom:1px solid #0f3460;flex-wrap:wrap}
.air-note{font-weight:bold;min-width:40px;font-size:0.85em}
.air-sliders{flex:1;min-width:150px}
.air-vals{font-size:0.75em;color:#9aa;display:flex;justify-content:space-between}
.dual-range{position:relative;height:28px;margin:4px 0}
.dual-range-track{position:absolute;top:12px;left:0;right:0;height:4px;background:#0f3460;border-radius:2px}
.dual-range-fill{position:absolute;top:12px;height:4px;background:#e94560;border-radius:2px}
.dual-range input[type=range]{position:absolute;top:0;width:100%;margin:0;pointer-events:none;
-webkit-appearance:none;appearance:none;background:transparent;height:28px}
.dual-range input[type=range]::-webkit-slider-thumb{pointer-events:all;-webkit-appearance:none;
width:16px;height:16px;background:#e94560;border-radius:50%;cursor:pointer;border:2px solid #fff}
.dual-range input[type=range]::-moz-range-thumb{pointer-events:all;width:16px;height:16px;
background:#e94560;border-radius:50%;cursor:pointer;border:2px solid #fff}
.undo-bar{display:flex;gap:6px;align-items:center;margin-bottom:8px}
.undo-bar button{padding:4px 10px;font-size:.8em}.undo-bar span{font-size:.75em;color:#777}
.settings-overlay{display:none;position:fixed;top:0;left:0;right:0;bottom:0;
background:rgba(0,0,0,.85);z-index:100;overflow-y:auto}
.settings-overlay.open{display:block}
.settings-box{max-width:600px;margin:0 auto;padding:16px;padding-bottom:48px}
.settings-box h2{color:#e94560;margin-bottom:12px;display:flex;justify-content:space-between;align-items:center}
.close-btn{background:none;border:none;color:#9aa;font-size:1.5em;cursor:pointer}
.close-btn:hover{color:#e94560}
.drop-zone{border:2px dashed #0f3460;border-radius:8px;padding:30px;text-align:center;
color:#777;cursor:pointer;transition:border-color .2s}
.drop-zone.hover{border-color:#e94560;color:#e94560}
.transport{display:flex;gap:8px;justify-content:center;align-items:center;margin:12px 0}
.transport button{width:44px;height:44px;border-radius:50%;font-size:1.2em;
display:flex;align-items:center;justify-content:center}
.progress-bar{height:6px;background:#0f3460;border-radius:3px;overflow:hidden;margin:8px 0}
.progress-fill{height:100%;background:#e94560;width:0%;transition:width .3s}
.file-info{font-size:0.8em;color:#9aa;text-align:center}
.upload-bar{height:4px;background:#0f3460;border-radius:2px;overflow:hidden;margin-top:8px;display:none}
.upload-fill{height:100%;background:#4ecca3;width:0%;transition:width .15s}
.cc-bar{display:flex;align-items:center;gap:8px;margin-bottom:6px;font-size:0.8em}
.cc-label{min-width:70px;color:#9aa}.cc-val{min-width:24px;text-align:right}
.cc-track{flex:1;height:6px;background:#0f3460;border-radius:3px;overflow:hidden}
.cc-fill{height:100%;background:#4ecca3;transition:width .2s}
.vu{display:flex;align-items:center;gap:8px;margin:8px 0}
.vu-track{flex:1;height:10px;background:#0f3460;border-radius:5px;overflow:hidden}
.vu-fill{height:100%;background:#4ecca3;width:0%;transition:width .1s}
.vu-val{font-size:0.8em;min-width:36px;text-align:right}
.mic-badge{display:inline-block;background:#4ecca3;color:#1a1a2e;font-size:0.7em;
padding:2px 8px;border-radius:10px;font-weight:bold}
.mic-badge.off{background:#777;color:#9aa}
.pitch{display:flex;gap:16px;align-items:center;font-size:0.9em}
.pitch-note{font-size:1.4em;font-weight:bold;color:#e94560;min-width:50px}
.pitch-hz{color:#9aa;font-size:0.85em}
.pitch-cents{font-size:0.85em}.pitch-cents.ok{color:#4ecca3}.pitch-cents.sharp{color:#e94560}.pitch-cents.flat{color:#e9a645}
.acal-progress{background:#0d1b3e;border-radius:8px;padding:12px;display:none}
.acal-bar{height:8px;background:#0f3460;border-radius:4px;overflow:hidden;margin:6px 0}
.acal-fill{height:100%;background:#e94560;width:0%;transition:width .3s}
.acal-info{font-size:0.8em;color:#9aa;display:flex;justify-content:space-between}
.wifi-item{padding:8px;border-bottom:1px solid #0f3460;cursor:pointer;display:flex;justify-content:space-between}
.wifi-item:hover{background:#0f3460}
.status-bar{background:#0d1117;padding:6px 16px;font-size:0.75em;color:#777;
display:flex;justify-content:space-between;position:fixed;bottom:0;left:0;right:0;z-index:5}
.log{background:#0a0a1a;border-radius:4px;padding:8px;font-family:monospace;font-size:0.75em;
max-height:120px;overflow-y:auto;color:#9aa}
</style>
</head>
<body>
<div class="toast-container" id="toastContainer"></div>
<div class="hdr">
  <h1 id="devName">ServoFlute<span class="unsaved-badge" id="unsavedBadge">modifi&eacute;</span></h1>
  <div class="hdr-r">
    <span class="dot off" id="sDot"></span>
    <button class="gear-btn" onclick="toggleSettings()" title="Reglages" id="gearBtn">
      <svg viewBox="0 0 16 16" width="18" height="18"><circle cx="8" cy="8" r="2" fill="currentColor"/><path d="M14.3 6.7l-1.2-.2a5.2 5.2 0 00-.5-1.1l.7-1-1.7-1.7-1 .7c-.3-.2-.7-.4-1.1-.5L9.3 1.7H7.7l-.2 1.2c-.4.1-.8.3-1.1.5l-1-.7L3.7 4.4l.7 1c-.2.3-.4.7-.5 1.1L2.7 6.7v1.6l1.2.2c.1.4.3.8.5 1.1l-.7 1 1.7 1.7 1-.7c.3.2.7.4 1.1.5l.2 1.2h1.6l.2-1.2c.4-.1.8-.3 1.1-.5l1 .7 1.7-1.7-.7-1c.2-.3.4-.7.5-1.1l1.2-.2V6.7z" fill="none" stroke="currentColor" stroke-width="1.2"/></svg>
    </button>
  </div>
</div>

<div class="tabs">
  <button class="active" onclick="showTab('keyboard',this)"><svg viewBox="0 0 16 14" width="14" height="12"><rect x="1" y="1" width="14" height="12" rx="2" fill="none" stroke="currentColor" stroke-width="1.2"/><rect x="3" y="4" width="2" height="2" rx=".5" fill="currentColor"/><rect x="7" y="4" width="2" height="2" rx=".5" fill="currentColor"/><rect x="11" y="4" width="2" height="2" rx=".5" fill="currentColor"/><rect x="4" y="8" width="8" height="2" rx=".5" fill="currentColor"/></svg>Clavier</button>
  <button onclick="showTab('midi',this)"><svg viewBox="0 0 14 16" width="12" height="14"><path d="M12 1v10.5a2.5 2.5 0 11-2-2.45V3.5L5 5v8a2.5 2.5 0 11-2-2.45V1l9-2z" fill="currentColor" opacity=".85"/></svg>MIDI</button>
  <button onclick="showTab('calib',this)"><svg viewBox="0 0 16 16" width="14" height="14"><path d="M6.5 1L7 4H5L2 8h3l-.5 7 6-9H7.5l2-5z" fill="currentColor" opacity=".85"/></svg>Calibration</button>
</div>

<!-- TAB: KEYBOARD -->
<div class="tab active" id="tab-keyboard">
  <div class="flute-box">
    <svg id="fluteSvg" viewBox="0 0 400 100"></svg>
    <div class="flute-info"><span id="fluteNote">-</span> <span id="fluteInfo" style="color:#555">Jouez une note</span></div>
  </div>
  <div class="section">
    <div class="cfg-row"><label>Velocity</label>
      <input type="range" min="1" max="127" value="100" id="velSlider" oninput="setVelocity(this.value)">
      <span id="velVal" style="min-width:28px;text-align:right"></span>
    </div>
    <div class="cfg-row"><label>Souffle</label>
      <input type="range" min="0" max="100" value="50" id="airSlider" oninput="setAirLive(this.value)">
      <span id="airVal" style="min-width:36px;text-align:right">50%</span>
    </div>
  </div>
  <div class="keys" id="pianoKeys"></div>
</div>

<!-- TAB: MIDI PLAYER -->
<div class="tab" id="tab-midi">
  <div class="section">
    <h3>Fichier MIDI</h3>
    <div class="drop-zone" id="dropZone" onclick="document.getElementById('midiFile').click()">
      Glisser-deposer un fichier MIDI ou cliquer
      <input type="file" id="midiFile" accept=".mid,.midi" style="display:none" onchange="uploadMidi(this)">
    </div>
    <div class="upload-bar" id="uploadBar"><div class="upload-fill" id="uploadFill"></div></div>
    <div class="file-info" id="fileInfo" style="margin-top:8px">
      <span id="fName"></span> &bull; <span id="fEvents"></span> evt &bull; <span id="fDuration"></span>
    </div>
  </div>
  <div class="section">
    <div class="transport">
      <button class="btn btn-g" id="btnPlay" onclick="wsSend({t:'play'})" disabled><svg viewBox="0 0 16 16" width="18" height="18"><path d="M4 2l10 6-10 6z" fill="currentColor"/></svg></button>
      <button class="btn btn-s" id="btnPause" onclick="wsSend({t:'pause'})" disabled><svg viewBox="0 0 16 16" width="18" height="18"><rect x="3" y="2" width="3.5" height="12" rx="1" fill="currentColor"/><rect x="9.5" y="2" width="3.5" height="12" rx="1" fill="currentColor"/></svg></button>
      <button class="btn btn-p" id="btnStop" onclick="wsSend({t:'stop'})" disabled><svg viewBox="0 0 16 16" width="18" height="18"><rect x="3" y="3" width="10" height="10" rx="1" fill="currentColor"/></svg></button>
    </div>
    <div class="progress-bar"><div class="progress-fill" id="progressFill"></div></div>
    <div class="file-info" id="progressText">--:-- / --:--</div>
  </div>
</div>

<!-- TAB: CALIBRATION -->
<div class="tab" id="tab-calib">
  <div class="steps">
    <div class="step-dot active" onclick="goStep(1)"></div>
    <div class="step-line"></div>
    <div class="step-dot" onclick="goStep(2)"></div>
    <div class="step-line"></div>
    <div class="step-dot" onclick="goStep(3)"></div>
  </div>
  <div class="step-labels">
    <span>Doigts</span><span>Doigtes</span><span>Souffle</span>
  </div>

  <!-- STEP 1: FINGERS -->
  <div id="step1" class="step-panel">
    <div class="section">
      <h3>Instrument</h3>
      <div class="cfg-row"><label>Type d'instrument</label>
        <select id="instrumentSelect" style="flex:1;max-width:260px" onchange="selectInstrument(this.value)"></select>
      </div>
    </div>
    <div class="section">
      <h3>Configuration des servos</h3>
      <div class="cfg-row"><label>Nombre de doigts</label>
        <div style="display:flex;align-items:center;gap:8px">
          <button class="btn btn-s" onclick="changeFingers(-1)">-</button>
          <span id="numFingersDisp" style="min-width:24px;text-align:center;font-weight:bold">6</span>
          <button class="btn btn-s" onclick="changeFingers(1)">+</button>
        </div>
      </div>
      <div class="cfg-row"><label>Servo airflow PCA</label>
        <select id="airPca" style="max-width:80px" onchange="checkPca()"></select>
      </div>
      <div class="cfg-row"><label>Amplitude ouverture</label>
        <input type="range" min="10" max="90" value="30" id="angleOpen" oninput="$('aoVal').textContent=this.value+'&deg;'">
        <span id="aoVal" style="min-width:36px">30&deg;</span>
      </div>
    </div>
    <div class="flute-box">
      <svg id="calFluteSvg" viewBox="0 0 400 100"></svg>
    </div>
    <div id="fingerCards"></div>
    <div class="btn-row" style="justify-content:flex-end">
      <button class="btn btn-p" id="btnSaveStep1" onclick="saveStep1()"><svg viewBox="0 0 16 16" width="14" height="14"><path d="M12.7 1H3a2 2 0 00-2 2v10a2 2 0 002 2h10a2 2 0 002-2V3.3L12.7 1zM8 13a2.5 2.5 0 110-5 2.5 2.5 0 010 5zM11 5H5V2h6v3z" fill="currentColor"/></svg>Sauver &amp; Continuer &rarr;</button>
    </div>
  </div>

  <!-- STEP 2: FINGERINGS -->
  <div id="step2" class="step-panel" style="display:none">
    <div class="section">
      <h3>Notes &amp; doigtes</h3>
      <p style="font-size:.8em;color:#888;margin:0 0 8px">Choisissez un jeu de doigtes pour definir quelles notes l'instrument peut jouer et comment les jouer.</p>
      <div class="cfg-row"><label>Doigtes</label>
        <select id="presetSelect" style="flex:1;max-width:320px" onchange="applyPreset(this.value);updPresetInfo()"></select>
      </div>
    </div>
    <div class="section" id="fingeringSection">
      <div style="display:flex;gap:12px;font-size:.75em;color:#888;margin-bottom:8px">
        <span><span class="fg-dot open" style="display:inline-block;width:12px;height:12px;vertical-align:middle"></span> Ouvert</span>
        <span><span class="fg-dot closed" style="display:inline-block;width:12px;height:12px;vertical-align:middle"></span> Ferme</span>
        <span><span class="fg-dot closed thumb" style="display:inline-block;width:12px;height:12px;vertical-align:middle"></span> Pouce</span>
      </div>
      <div class="undo-bar">
        <button class="btn btn-s" id="undoBtn" onclick="undoFp()" disabled title="Ctrl+Z"><svg viewBox="0 0 16 16" width="12" height="12"><path d="M4 7h8a3 3 0 010 6H9" fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round"/><path d="M7 4L4 7l3 3" fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"/></svg>Annuler</button>
        <button class="btn btn-s" id="redoBtn" onclick="redoFp()" disabled title="Ctrl+Y"><svg viewBox="0 0 16 16" width="12" height="12"><path d="M12 7H4a3 3 0 000 6h3" fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round"/><path d="M9 4l3 3-3 3" fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"/></svg>Retablir</button>
        <span id="undoInfo"></span>
      </div>
      <div id="fingeringRows"></div>
      <div class="btn-row">
        <button class="btn btn-s" onclick="addNote()">+ Ajouter note</button>
        <button class="btn btn-s" onclick="removeLastNote()">- Supprimer</button>
      </div>
    </div>
    <div class="btn-row" style="justify-content:space-between">
      <button class="btn btn-s" onclick="goStep(1)"><svg viewBox="0 0 16 16" width="14" height="14"><path d="M10 3L5 8l5 5" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/></svg>Retour</button>
      <button class="btn btn-p" id="btnSaveStep2" onclick="saveStep2()"><svg viewBox="0 0 16 16" width="14" height="14"><path d="M12.7 1H3a2 2 0 00-2 2v10a2 2 0 002 2h10a2 2 0 002-2V3.3L12.7 1zM8 13a2.5 2.5 0 110-5 2.5 2.5 0 010 5zM11 5H5V2h6v3z" fill="currentColor"/></svg>Sauver &amp; Continuer &rarr;</button>
    </div>
  </div>

  <!-- STEP 3: AIRFLOW -->
  <div id="step3" class="step-panel" style="display:none">
    <div class="section" id="micSection" style="display:none">
      <h3><span class="mic-badge" id="micBadge">MIC</span> Auto-calibration</h3>
      <div class="vu"><span style="font-size:.8em;min-width:24px">VU</span>
        <div class="vu-track"><div class="vu-fill" id="vuFill"></div></div>
        <span class="vu-val" id="vuVal">0%</span>
      </div>
      <div class="pitch">
        <span class="pitch-note" id="pitchNote">-</span>
        <span class="pitch-hz" id="pitchHz">- Hz</span>
        <span class="pitch-cents ok" id="pitchCents">-</span>
      </div>
      <div class="btn-row">
        <button class="btn btn-g" id="btnAcalStart" onclick="startAutoCal()">Auto-calibrer toutes les notes</button>
        <button class="btn btn-p" id="btnAcalStop" onclick="stopAutoCal()" style="display:none">Arreter</button>
      </div>
      <div class="acal-progress" id="acalProgress">
        <div class="acal-info"><span id="acalStep">-</span><span id="acalState">-</span></div>
        <div class="acal-bar"><div class="acal-fill" id="acalFill"></div></div>
        <div id="acalResults" style="margin-top:8px;display:none"></div>
      </div>
    </div>
    <div class="section">
      <h3>Souffle par note</h3>
      <div id="airflowRows"></div>
    </div>
    <div class="btn-row" style="justify-content:space-between">
      <button class="btn btn-s" onclick="goStep(2)"><svg viewBox="0 0 16 16" width="14" height="14"><path d="M10 3L5 8l5 5" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/></svg>Retour</button>
      <button class="btn btn-g" id="btnSaveStep3" onclick="saveStep3()"><svg viewBox="0 0 16 16" width="14" height="14"><path d="M3 8l3.5 4L13 4" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/></svg>Sauver &amp; Terminer</button>
    </div>
  </div>
</div>

<!-- SETTINGS OVERLAY -->
<div class="settings-overlay" id="settingsOverlay">
<div class="settings-box">
  <h2>Reglages <button class="close-btn" onclick="toggleSettings()">&times;</button></h2>

  <div class="section"><h3>Appareil</h3>
    <div class="cfg-row"><label>Nom</label><input type="text" id="cfgDevice" maxlength="31"></div>
    <div class="cfg-row"><label>Canal MIDI</label><select id="cfgMidiCh">
      <option value="0">Omni (tous)</option>
    </select></div>
  </div>

  <div class="section"><h3>Timing</h3>
    <div class="cfg-row"><label>Servo&rarr;valve (ms)</label><input type="number" id="cfgDelay" min="0" max="1000"></div>
    <div class="cfg-row"><label>Intervalle valve (ms)</label><input type="number" id="cfgValveInt" min="0" max="500"></div>
    <div class="cfg-row"><label>Note min (ms)</label><input type="number" id="cfgMinNote" min="0" max="500"></div>
  </div>

  <div class="section"><h3>Servo Airflow</h3>
    <div class="cfg-row"><label>Angle repos</label><input type="number" id="cfgAirOff" min="0" max="180"></div>
    <div class="cfg-row"><label>Angle min</label><input type="number" id="cfgAirMin" min="0" max="180"></div>
    <div class="cfg-row"><label>Angle max</label><input type="number" id="cfgAirMax" min="0" max="180"></div>
  </div>

  <div class="section"><h3>Vibrato</h3>
    <div class="cfg-row"><label>Frequence (Hz)</label><input type="number" id="cfgVibF" min="0" max="20" step="0.5"></div>
    <div class="cfg-row"><label>Amplitude (deg)</label><input type="number" id="cfgVibA" min="0" max="30" step="0.5"></div>
  </div>

  <div class="section"><h3>CC Defaults</h3>
    <div class="cfg-row"><label>Volume (CC7)</label><input type="number" id="cfgCCVol" min="0" max="127"></div>
    <div class="cfg-row"><label>Expression (CC11)</label><input type="number" id="cfgCCExpr" min="0" max="127"></div>
    <div class="cfg-row"><label>Modulation (CC1)</label><input type="number" id="cfgCCMod" min="0" max="127"></div>
    <div class="cfg-row"><label>Breath (CC2)</label><input type="number" id="cfgCCBreath" min="0" max="127"></div>
    <div class="cfg-row"><label>Brightness (CC74)</label><input type="number" id="cfgCCBright" min="0" max="127"></div>
  </div>

  <div class="section"><h3>Breath CC2</h3>
    <div class="cfg-row"><label>Active</label><input type="checkbox" id="cfgCC2On" style="width:auto;flex:0"></div>
    <div class="cfg-row"><label>Seuil silence</label><input type="number" id="cfgCC2Thr" min="0" max="127"></div>
    <div class="cfg-row"><label>Courbe</label><input type="number" id="cfgCC2Curve" min="0.1" max="5" step="0.1"></div>
    <div class="cfg-row"><label>Timeout (ms)</label><input type="number" id="cfgCC2To" min="0" max="10000"></div>
  </div>

  <div class="section"><h3>Solenoide PWM</h3>
    <div class="cfg-row"><label>PWM activation</label><input type="number" id="cfgSolAct" min="0" max="255"></div>
    <div class="cfg-row"><label>PWM maintien</label><input type="number" id="cfgSolHold" min="0" max="255"></div>
    <div class="cfg-row"><label>Temps (ms)</label><input type="number" id="cfgSolTime" min="0" max="500"></div>
  </div>

  <div class="section"><h3>Power</h3>
    <div class="cfg-row"><label>Timeout (ms)</label><input type="number" id="cfgUnpower" min="0" max="60000"></div>
  </div>

  <div class="section"><h3>WiFi</h3>
    <div class="cfg-row"><label>Etat</label><span id="wifiState" style="font-size:.85em;color:#888">-</span></div>
    <div class="btn-row"><button class="btn btn-s" onclick="startWifiScan()">Scanner</button>
      <span style="font-size:.75em;color:#555" id="scanStatus"></span></div>
    <div id="wifiList" style="margin:8px 0"></div>
    <div class="cfg-row"><label>SSID</label><input type="text" id="wifiSsid" maxlength="32"></div>
    <div class="cfg-row"><label>Mot de passe</label><input type="password" id="wifiPass" maxlength="64"></div>
    <div class="btn-row"><button class="btn btn-g" onclick="connectWifi()">Connecter</button></div>
    <div style="font-size:.75em;color:#888;margin-top:4px" id="wifiMsg"></div>
  </div>

  <div class="section"><h3>Monitor</h3>
    <div style="display:flex;gap:16px;margin-bottom:8px">
      <div><span style="color:#888;font-size:.8em">Etat</span><div style="font-weight:bold" id="monState">IDLE</div></div>
      <div><span style="color:#888;font-size:.8em">Heap</span><div style="font-weight:bold" id="monHeap">-</div></div>
    </div>
    <div class="cc-bar"><span class="cc-label">CC1 Mod</span><div class="cc-track"><div class="cc-fill" id="ccBar1"></div></div><span class="cc-val" id="ccV1">0</span></div>
    <div class="cc-bar"><span class="cc-label">CC2 Breath</span><div class="cc-track"><div class="cc-fill" id="ccBar2"></div></div><span class="cc-val" id="ccV2">127</span></div>
    <div class="cc-bar"><span class="cc-label">CC7 Vol</span><div class="cc-track"><div class="cc-fill" id="ccBar7"></div></div><span class="cc-val" id="ccV7">127</span></div>
    <div class="cc-bar"><span class="cc-label">CC11 Expr</span><div class="cc-track"><div class="cc-fill" id="ccBar11"></div></div><span class="cc-val" id="ccV11">127</span></div>
    <div class="log" id="logBox"></div>
    <div class="btn-row"><button class="btn btn-p" onclick="wsSend({t:'panic'})">ALL SOUND OFF</button></div>
  </div>

  <div class="btn-row" style="justify-content:center;margin-top:16px">
    <button class="btn btn-g" id="btnSaveSettings" onclick="saveSettings()"><svg viewBox="0 0 16 16" width="14" height="14"><path d="M12.7 1H3a2 2 0 00-2 2v10a2 2 0 002 2h10a2 2 0 002-2V3.3L12.7 1zM8 13a2.5 2.5 0 110-5 2.5 2.5 0 010 5zM11 5H5V2h6v3z" fill="currentColor"/></svg>Sauvegarder</button>
    <button class="btn btn-s" onclick="resetConfig()">Reset defauts</button>
  </div>
  <div style="font-size:.75em;color:#9aa;text-align:center;margin-top:8px" id="settingsMsg"></div>
</div>
</div>

<div class="status-bar">
  <span id="sText">Deconnecte</span>
  <span id="heapBar">-</span>
</div>

<script>
// --- Constants (mirrored from settings.h) ---
const MIDI_CC_MAX=127,MIDI_VEL_MAX=127,MAX_FINGERS=15;
const WEB_DEF_VEL=100,TEST_SOL_MS=2000,VU_SCALE=500,PITCH_OK_CT=15;

const N=['C','C#','D','D#','E','F','F#','G','G#','A','A#','B'];
const STATES=['IDLE','POSITIONING','PLAYING','STOPPING'];
let ws=null,velocity=WEB_DEF_VEL,CFG=null,curNote=null;
let calibStep=1,fileLoaded=false,playerDuration=0;
let micDetected=false,autoCalRunning=false;
let dirty=false,fpHistory=[],fpFuture=[];

// Presets: {id,n:name,h:holes,th:thumbIdx(-1=none),d:[[midi,[fp],amn,amx],...]}
const PR=[
{id:'tabor_d',n:'Tabor Pipe (R\u00e9)',h:3,th:0,d:[
[74,[0,0,0],10,50],[76,[0,0,1],10,50],[78,[0,1,1],10,55],[79,[1,1,1],10,55],
[81,[0,0,0],30,70],[83,[0,0,1],30,70],[85,[0,1,1],30,75],[86,[1,1,1],35,75]]},
{id:'ocarina_c',n:'Ocarina 4 trous (Do)',h:4,th:-1,d:[
[72,[0,0,0,0],10,50],[74,[0,0,1,0],10,55],[76,[0,1,0,0],15,60],[77,[0,1,0,1],20,65],
[79,[0,1,1,1],25,70],[81,[1,0,1,0],30,75],[83,[1,1,1,0],35,85],[84,[1,1,1,1],35,90]]},
{id:'naf_a',n:'Fl\u00fbte am\u00e9rindienne (La)',h:4,th:-1,d:[
[69,[0,0,0,0],5,45],[72,[0,0,0,1],5,45],[74,[0,0,1,1],5,50],[76,[0,1,1,1],5,50],[79,[1,1,1,1],5,55],
[81,[0,0,0,0],40,80],[84,[0,0,0,1],40,80],[86,[0,0,1,1],40,85],[88,[0,1,1,1],45,85]]},
{id:'shaku_d',n:'Shakuhachi (R\u00e9)',h:5,th:0,d:[
[62,[0,0,0,0,0],5,40],[65,[0,0,0,1,0],5,45],[67,[0,0,1,1,1],5,50],[69,[0,1,1,1,1],10,50],[72,[1,1,1,1,0],10,55],
[74,[0,0,0,0,0],40,80],[77,[0,0,0,1,0],40,80],[79,[0,0,1,1,1],40,85],[81,[0,1,1,1,1],45,85],[84,[1,1,1,1,0],45,90],[86,[0,0,0,0,0],60,100]]},
{id:'naf5_fs',n:'Fl\u00fbte am\u00e9rindienne 5 (Fa#)',h:5,th:-1,d:[
[66,[0,0,0,0,0],5,45],[69,[0,0,0,0,1],5,45],[71,[0,0,0,1,1],5,50],[73,[0,0,1,1,1],5,50],[76,[0,1,1,1,1],5,55],[78,[1,1,1,1,1],5,55],
[78,[0,0,0,0,0],35,75],[81,[0,0,0,0,1],35,75],[83,[0,0,0,1,1],35,80],[85,[0,0,1,1,1],40,80],[88,[0,1,1,1,1],40,85]]},
{id:'whistle_d',n:'Tin Whistle (R\u00e9)',h:6,th:-1,d:[
[74,[0,0,0,0,0,0],5,50],[76,[0,0,0,0,0,1],5,50],[78,[0,0,0,0,1,1],5,50],[79,[0,0,0,1,1,1],5,55],
[81,[0,0,1,1,1,1],5,55],[83,[0,1,1,1,1,1],5,60],[85,[1,1,1,1,1,1],5,60],
[86,[0,0,0,0,0,0],30,80],[88,[0,0,0,0,0,1],30,80],[90,[0,0,0,0,1,1],30,85],[91,[0,0,0,1,1,1],35,85],
[93,[0,0,1,1,1,1],35,90],[95,[0,1,1,1,1,1],35,90],[97,[1,1,1,1,1,1],40,95]]},
{id:'irish_c',n:'Fl\u00fbte irlandaise (Do)',h:6,th:-1,d:[
[82,[0,1,1,1,1,1],10,60],[83,[1,1,1,1,1,1],0,50],
[84,[0,0,0,0,0,0],20,75],[86,[0,0,0,0,0,1],15,70],[88,[0,0,0,0,1,1],10,65],[89,[0,0,0,1,1,1],10,60],
[91,[0,0,1,1,1,1],5,55],[93,[0,1,1,1,1,1],5,50],[95,[1,1,1,1,1,1],0,45],
[96,[0,0,0,0,0,0],50,100],[98,[0,0,0,0,0,1],45,95],[100,[0,0,0,0,1,1],40,90],[101,[0,0,0,1,1,1],35,85],
[103,[0,0,1,1,1,1],30,80]]},
{id:'bansuri_a',n:'Bansuri (La)',h:6,th:-1,d:[
[64,[0,0,0,0,0,0],5,45],[66,[0,0,0,0,0,1],5,45],[68,[0,0,0,0,1,1],5,50],
[69,[0,0,0,1,1,1],10,50],[71,[0,0,1,1,1,1],10,55],[73,[0,1,1,1,1,1],10,55],
[74,[1,0,0,0,0,0],15,60],[76,[0,0,0,0,0,0],20,65],[78,[0,0,0,0,0,1],20,65],[80,[0,0,0,0,1,1],20,70],
[81,[0,0,0,1,1,1],35,80],[83,[0,0,1,1,1,1],35,85],[85,[0,1,1,1,1,1],40,85]]},
{id:'dizi_d',n:'Dizi (R\u00e9)',h:6,th:-1,d:[
[69,[0,0,0,0,0,0],5,45],[71,[0,0,0,0,0,1],5,45],[73,[0,0,0,0,1,1],5,50],
[74,[0,0,0,1,1,1],10,50],[76,[0,0,1,1,1,1],10,55],[78,[0,1,1,1,1,1],10,55],
[81,[0,0,0,0,0,0],30,75],[83,[0,0,0,0,0,1],30,75],[85,[0,0,0,0,1,1],30,80],
[86,[0,0,0,1,1,1],35,80],[88,[0,0,1,1,1,1],35,85],[90,[0,1,1,1,1,1],40,85]]},
{id:'fife_bb',n:'Fifre (Sib)',h:6,th:-1,d:[
[70,[0,0,0,0,0,0],10,55],[72,[0,0,0,0,0,1],10,55],[74,[0,0,0,0,1,1],10,55],[75,[0,0,0,1,1,1],10,60],
[77,[0,0,1,1,1,1],10,60],[79,[0,1,1,1,1,1],10,60],[81,[1,1,1,1,1,1],10,65],
[82,[0,0,0,0,0,0],35,80],[84,[0,0,0,0,0,1],35,80],[86,[0,0,0,0,1,1],35,85],[87,[0,0,0,1,1,1],40,85],
[89,[0,0,1,1,1,1],40,90],[91,[0,1,1,1,1,1],40,90],[93,[1,1,1,1,1,1],45,95]]},
{id:'quena_g',n:'Quena (Sol)',h:7,th:0,d:[
[67,[0,0,0,0,0,0,0],5,45],[69,[0,0,0,0,0,0,1],5,45],[71,[0,0,0,0,0,1,1],5,50],[72,[0,0,0,0,1,1,1],5,50],
[74,[0,0,0,1,1,1,1],5,55],[76,[0,0,1,1,1,1,1],5,55],[78,[0,1,1,1,1,1,1],5,60],
[79,[1,0,0,0,0,0,0],25,70],[81,[1,0,0,0,0,0,1],25,70],[83,[1,0,0,0,0,1,1],25,75],[84,[1,0,0,0,1,1,1],30,75],
[86,[1,0,0,1,1,1,1],30,80],[88,[1,0,1,1,1,1,1],30,80],[90,[1,1,1,1,1,1,1],35,85]]},
{id:'ney_a',n:'Ney turc (La)',h:7,th:0,d:[
[57,[0,0,0,0,0,0,0],0,30],[59,[0,0,0,0,0,0,1],0,30],[60,[0,0,0,0,0,1,1],0,35],
[62,[0,0,0,0,1,1,1],0,35],[64,[0,0,0,1,1,1,1],0,40],[65,[0,0,1,1,1,1,1],0,40],[67,[0,1,1,1,1,1,1],0,45],
[69,[0,0,0,0,0,0,0],20,65],[71,[0,0,0,0,0,0,1],20,65],[72,[0,0,0,0,0,1,1],20,70],
[74,[0,0,0,0,1,1,1],25,70],[76,[0,0,0,1,1,1,1],25,75],[77,[0,0,1,1,1,1,1],25,75],[79,[0,1,1,1,1,1,1],30,80]]},
{id:'recorder_c',n:'Fl\u00fbte \u00e0 bec (Do)',h:7,th:0,d:[
[72,[0,0,0,0,0,0,0],5,40],[74,[0,0,0,0,0,0,1],5,40],[76,[0,0,0,0,0,1,1],5,45],[77,[0,0,0,0,1,1,1],5,45],
[79,[0,0,0,1,1,1,1],5,50],[81,[0,0,1,1,1,1,1],5,50],[83,[0,1,1,1,1,1,1],5,55],
[84,[1,0,0,0,0,0,0],20,65],[86,[1,0,0,0,0,0,1],20,65],[88,[1,0,0,0,0,1,1],25,70],[89,[1,0,0,0,1,1,1],25,70],
[91,[1,0,0,1,1,1,1],25,75],[93,[1,0,1,1,1,1,1],30,75],[95,[1,1,0,1,1,1,1],30,80],[96,[1,0,1,0,1,1,1],35,85]]},
{id:'recorder_b8',n:'Fl\u00fbte \u00e0 bec baroque (Do)',h:8,th:0,d:[
[72,[0,0,0,0,0,0,0,0],5,40],[74,[0,0,0,0,0,0,1,1],5,40],[76,[0,0,0,0,0,1,1,1],5,45],
[77,[0,0,0,0,1,0,0,1],5,45],[79,[0,0,0,1,1,1,1,1],5,50],[81,[0,0,1,1,1,1,1,1],10,55],
[83,[0,1,0,1,1,1,1,1],10,55],
[84,[1,0,0,1,1,1,1,1],15,60],[86,[1,0,0,0,0,0,1,1],25,70],[88,[1,0,0,0,0,1,1,1],25,70],
[89,[1,0,0,0,1,0,0,1],30,75],[91,[1,0,0,1,1,1,1,1],30,80],[93,[1,0,1,0,1,1,1,1],35,80],
[95,[1,1,0,1,0,1,1,1],35,85],[96,[1,0,1,0,1,0,1,1],40,90]]},
{id:'kaval_d',n:'Kaval (R\u00e9)',h:8,th:0,d:[
[62,[0,0,0,0,0,0,0,0],0,30],[64,[0,0,0,0,0,0,0,1],0,30],[65,[0,0,0,0,0,0,1,1],0,35],
[67,[0,0,0,0,0,1,1,1],0,35],[69,[0,0,0,0,1,1,1,1],0,40],[71,[0,0,0,1,1,1,1,1],5,40],
[72,[0,0,1,1,1,1,1,1],5,45],[74,[0,1,1,1,1,1,1,1],5,45],
[76,[0,0,0,0,0,0,0,1],25,65],[77,[0,0,0,0,0,0,1,1],25,70],
[79,[0,0,0,0,0,1,1,1],25,70],[81,[0,0,0,0,1,1,1,1],30,75],[83,[0,0,0,1,1,1,1,1],30,75],
[84,[0,0,1,1,1,1,1,1],30,80],[86,[0,1,1,1,1,1,1,1],35,80]]}
];

function showToast(msg,type){type=type||'info';const c=$('toastContainer');
  const ic={success:'<svg viewBox="0 0 16 16" width="16" height="16"><path d="M3 8l3.5 4L13 4" fill="none" stroke="#fff" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"/></svg>',
  error:'<svg viewBox="0 0 16 16" width="16" height="16"><path d="M5.5 5.5l5 5M10.5 5.5l-5 5" stroke="#fff" stroke-width="1.5" stroke-linecap="round"/></svg>',
  info:'<svg viewBox="0 0 16 16" width="16" height="16"><circle cx="8" cy="8" r="6" fill="none" stroke="#fff" stroke-width="1.5"/><path d="M8 7v4M8 5v.5" stroke="#fff" stroke-width="1.5" stroke-linecap="round"/></svg>'};
  const t=document.createElement('div');t.className='toast '+type;
  t.innerHTML=(ic[type]||ic.info)+'<span>'+esc(msg)+'</span>';c.appendChild(t);
  requestAnimationFrame(()=>requestAnimationFrame(()=>t.classList.add('show')));
  setTimeout(()=>{t.classList.remove('show');setTimeout(()=>t.remove(),300)},3000)}
function markDirty(){dirty=true;$('unsavedBadge').classList.add('show')}
function markClean(){dirty=false;$('unsavedBadge').classList.remove('show')}
function btnLoad(id,on){const b=$(id);if(!b)return;if(on){b.classList.add('loading');b.disabled=true}else{b.classList.remove('loading');b.disabled=false}}
function testPulse(el){el.classList.add('test-pulse');setTimeout(()=>el.classList.remove('test-pulse'),600)}
function fpSnap(){if(!CFG)return;fpHistory.push(JSON.stringify(CFG.notes.map(n=>({midi:n.midi,fp:[...n.fp]}))));
  fpFuture=[];if(fpHistory.length>50)fpHistory.shift();updUndoUI()}
function undoFp(){if(!fpHistory.length||!CFG)return;
  fpFuture.push(JSON.stringify(CFG.notes.map(n=>({midi:n.midi,fp:[...n.fp]}))));
  const s=JSON.parse(fpHistory.pop());s.forEach((sn,i)=>{if(CFG.notes[i]){CFG.notes[i].midi=sn.midi;CFG.notes[i].fp=sn.fp}});
  buildFingeringRows();updUndoUI();markDirty()}
function redoFp(){if(!fpFuture.length||!CFG)return;
  fpHistory.push(JSON.stringify(CFG.notes.map(n=>({midi:n.midi,fp:[...n.fp]}))));
  const s=JSON.parse(fpFuture.pop());s.forEach((sn,i)=>{if(CFG.notes[i]){CFG.notes[i].midi=sn.midi;CFG.notes[i].fp=sn.fp}});
  buildFingeringRows();updUndoUI();markDirty()}
function updUndoUI(){$('undoBtn').disabled=!fpHistory.length;$('redoBtn').disabled=!fpFuture.length;
  $('undoInfo').textContent=fpHistory.length?fpHistory.length+' modif.':''}
function checkPca(){if(!CFG)return;const used={};const airP=parseInt($('airPca').value);used[airP]='Souffle';
  document.querySelectorAll('.cal-card').forEach((card,i)=>{const ch=CFG.fingers[i]?CFG.fingers[i].ch:i;
    let conflict=used[ch]!==undefined;card.classList.toggle('pca-conflict',conflict);
    const w=card.querySelector('.pca-warn');if(w)w.textContent=conflict?'Conflit PCA '+ch+' avec '+used[ch]:'';
    used[ch]='Doigt '+(i+1)})}
function updDualFill(ni){if(!CFG)return;
  if(CFG.notes[ni].amn>CFG.notes[ni].amx){const t=CFG.notes[ni].amn;CFG.notes[ni].amn=CFG.notes[ni].amx;CFG.notes[ni].amx=t;
    const mi=$('amn'+ni),mx=$('amx'+ni);if(mi)mi.textContent=CFG.notes[ni].amn;if(mx)mx.textContent=CFG.notes[ni].amx}
  const f=$('drf'+ni);if(!f)return;const a=CFG.notes[ni].amn,b=CFG.notes[ni].amx;
  f.style.left=a+'%';f.style.width=(b-a)+'%'}

function mn(m){return N[m%12]+(Math.floor(m/12)-1)}
function isBlack(m){return[1,3,6,8,10].includes(m%12)}
function $(id){return document.getElementById(id)}
function esc(s){return String(s).replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;').replace(/"/g,'&quot;')}
function fmt(ms){if(!ms||ms<=0)return'--:--';const s=Math.floor(ms/1000);return String(s/60|0).padStart(2,'0')+':'+String(s%60).padStart(2,'0')}
function addLog(t){const b=$('logBox');if(!b)return;b.innerHTML+='<div>'+esc(t)+'</div>';b.scrollTop=b.scrollHeight;
  while(b.children.length>100)b.removeChild(b.firstChild)}

// --- Tabs ---
function showTab(id,btn){
  document.querySelectorAll('.tab').forEach(t=>t.classList.remove('active'));
  document.querySelectorAll('.tabs button').forEach(b=>b.classList.remove('active'));
  $('tab-'+id).classList.add('active');if(btn)btn.classList.add('active');
  if(id==='calib'&&CFG)buildCalibUI();
}
function toggleSettings(){$('settingsOverlay').classList.toggle('open');if($('settingsOverlay').classList.contains('open')&&CFG)fillSettings()}

// --- WebSocket ---
let wsRetry=0;
function wsConnect(){
  const p=location.protocol==='https:'?'wss:':'ws:';
  ws=new WebSocket(p+'//'+location.host+'/ws');
  ws.onopen=()=>{wsRetry=0;$('sDot').className='dot on';$('sText').textContent='Connecte';addLog('WS connecte')};
  ws.onclose=()=>{$('sDot').className='dot off';$('sText').textContent='Deconnecte';
    const d=Math.min(30000,2000*Math.pow(1.5,wsRetry));wsRetry++;setTimeout(wsConnect,d)};
  ws.onerror=()=>{ws.close()};
  ws.onmessage=e=>{try{handleWs(JSON.parse(e.data))}catch(x){addLog('WS err: '+x.message)}};
}
function wsSend(o){if(ws&&ws.readyState===1)ws.send(JSON.stringify(o))}

function handleWs(d){
  if(d.t==='status'){
    $('monState').textContent=STATES[d.state]||'?';
    $('monState').style.color=d.playing?'#e94560':'#4ecca3';
    if(d.heap){$('monHeap').textContent=(d.heap/1024|0)+'KB';$('heapBar').textContent=(d.heap/1024|0)+'KB'}
    updateCC(1,d.cc1);updateCC(2,d.cc2);updateCC(7,d.cc7);updateCC(11,d.cc11);
    if(d.pp!==undefined)$('progressFill').style.width=d.pp+'%';
    if(d.ppos!==undefined)$('progressText').textContent=fmt(d.ppos)+' / '+fmt(playerDuration);
    if(d.ps!==undefined){$('btnPlay').disabled=(d.ps===1);$('btnPause').disabled=(d.ps!==1);$('btnStop').disabled=(d.ps===0&&!fileLoaded)}
  }else if(d.t==='midi_loaded'){
    fileLoaded=true;playerDuration=d.duration||0;
    $('fName').textContent=d.file;$('fEvents').textContent=d.events;$('fDuration').textContent=fmt(d.duration);
    $('btnPlay').disabled=false;$('btnStop').disabled=false;addLog('MIDI: '+d.file)
  }else if(d.t==='midi_error'){addLog('ERR: '+d.msg)}
  else if(d.t==='audio'){
    const rms=Math.min(100,Math.round((d.rms||0)*VU_SCALE));
    $('vuFill').style.width=rms+'%';$('vuVal').textContent=rms+'%';
    $('vuFill').style.background=rms>60?'#e94560':rms>30?'#e9a645':'#4ecca3';
    if(d.midi>0){$('pitchNote').textContent=mn(d.midi);$('pitchHz').textContent=Math.round(d.hz)+' Hz';
      const c=d.cents||0;$('pitchCents').textContent=(c>=0?'+':'')+c.toFixed(0)+' ct';
      $('pitchCents').className='pitch-cents '+(Math.abs(c)<PITCH_OK_CT?'ok':c>0?'sharp':'flat')}
    else{$('pitchNote').textContent='-';$('pitchHz').textContent='- Hz';$('pitchCents').textContent='-'}
  }else if(d.t==='acal_prog'){
    $('acalProgress').style.display='block';$('acalStep').textContent='Note '+(d.idx+1)+'/'+d.total+' '+d.note;
    $('acalFill').style.width=(((d.idx||0)/(d.total||1))*100)+'%';
    const sn={0:'Attente',1:'Prep',2:'Stab',3:'Sweep...',4:'Analyse',5:'OK'};
    $('acalState').textContent=sn[d.st]||'...'
  }else if(d.t==='acal_done'){
    autoCalRunning=false;$('btnAcalStart').style.display='';$('btnAcalStop').style.display='none';
    $('acalFill').style.width='100%';$('acalState').textContent='Termine !';addLog('Auto-cal OK');
    if(d.results){let h='';d.results.forEach(r=>{h+='<div style="display:flex;justify-content:space-between;font-size:.8em;padding:2px 0">'+
      '<span>'+esc(r.name)+'</span><span style="color:'+(r.ok?'#4ecca3':'#e94560')+'">'+(r.ok?esc(r.min+'%-'+r.max+'%'):'Echec')+'</span></div>'});
      $('acalResults').innerHTML=h;$('acalResults').style.display='block'}
    setTimeout(loadConfig,1000)
  }
}
function updateCC(n,v){if(v===undefined)return;const p=(v/MIDI_CC_MAX*100).toFixed(0);
  const b=$('ccBar'+n),t=$('ccV'+n);if(b)b.style.width=p+'%';if(t)t.textContent=v}

// --- Load config ---
function loadConfig(){
  const pk=$('pianoKeys');if(pk&&!CFG)pk.innerHTML='<div class="skeleton" style="width:100%;height:80px;margin:12px 0"></div>';
  fetch('/api/config').then(r=>r.json()).then(d=>{
    CFG=d;micDetected=d.mic||false;
    $('devName').childNodes[0].textContent=d.device||'ServoFlute';
    buildKeyboard();buildFlute(CFG,'fluteSvg',false);markClean();
    if(micDetected){$('micSection').style.display='';wsSend({t:'mic_mon',on:1})}
    else $('micSection').style.display='none';
  }).catch(e=>{addLog('Erreur config: '+e);showToast('Erreur chargement config','error')})
}

// --- KEYBOARD ---
function buildKeyboard(){
  const c=$('pianoKeys');c.innerHTML='';if(!CFG||!CFG.notes||!CFG.notes.length){c.innerHTML='<div style="color:#888;padding:16px;text-align:center">Aucune note</div>';return}
  CFG.notes.forEach((n,idx)=>{
    const name=mn(n.midi);const key=document.createElement('div');
    key.className='key'+(isBlack(n.midi)?' black':'')+' fade-in fade-delay-'+(Math.min(4,(idx%4)+1));key.dataset.midi=n.midi;
    let dots='<span class="kf-row">';for(let f=0;f<CFG.num_fingers;f++)dots+='<span class="kf '+(n.fp[f]?'o':'c')+'"></span>';dots+='</span>';
    const sc=idx<KC.length?KC[idx].toUpperCase():'';
    key.innerHTML='<span class="note-name">'+name+'</span><span class="note-midi">'+n.midi+'</span>'+dots+(sc?'<span class="key-shortcut">'+sc+'</span>':'');
    key.addEventListener('touchstart',e=>{e.preventDefault();noteOn(n.midi);key.classList.add('pressed')},{passive:false});
    key.addEventListener('touchend',e=>{e.preventDefault();noteOff(n.midi);key.classList.remove('pressed')},{passive:false});
    key.addEventListener('mousedown',e=>{e.preventDefault();noteOn(n.midi);key.classList.add('pressed')});
    key.addEventListener('mouseup',()=>{noteOff(n.midi);key.classList.remove('pressed')});
    key.addEventListener('mouseleave',()=>{if(key.classList.contains('pressed')){noteOff(n.midi);key.classList.remove('pressed')}});
    c.appendChild(key)
  });
  buildKeyMap()
}

function noteOn(midi){curNote=midi;updateFluteForNote(midi);
  document.querySelectorAll('#fluteSvg .flute-hole.open').forEach(h=>h.classList.add('playing'));
  // Update airflow slider range for this note
  if(CFG){const nd=CFG.notes.find(n=>n.midi===midi);if(nd){
    const sl=$('airSlider'),av=$('airVal');
    sl.min=nd.amn;sl.max=nd.amx;
    const mid=Math.round((nd.amn+nd.amx)/2);sl.value=mid;av.textContent=mid+'%';
    wsSend({t:'air_live',v:mid})}}
  wsSend({t:'non',n:midi,v:velocity})}
function noteOff(midi){wsSend({t:'nof',n:midi});if(curNote===midi){curNote=null;
  document.querySelectorAll('#fluteSvg .flute-hole.playing').forEach(h=>h.classList.remove('playing'))}}
function setVelocity(v){velocity=parseInt(v);$('velVal').textContent=v;wsSend({t:'velocity',v:velocity})}
function setAirLive(v){$('airVal').textContent=v+'%';wsSend({t:'air_live',v:parseInt(v)})}

// Keyboard shortcuts
const KC='azertyuiopqsdfghjklmwxcvbn'.split('');let keyMap={},keysDown=new Set();
function buildKeyMap(){keyMap={};if(!CFG)return;CFG.notes.forEach((n,i)=>{if(i<KC.length)keyMap[KC[i]]=n.midi})}
document.addEventListener('keydown',e=>{if(e.target.tagName==='INPUT'||e.target.tagName==='SELECT'||e.repeat||e.ctrlKey||e.altKey||e.metaKey)return;
  const n=keyMap[e.key.toLowerCase()];if(n&&!keysDown.has(e.key)){keysDown.add(e.key);noteOn(n);
    const el=document.querySelector('.key[data-midi="'+n+'"]');if(el)el.classList.add('pressed')}});
document.addEventListener('keyup',e=>{const n=keyMap[e.key.toLowerCase()];if(n){keysDown.delete(e.key);noteOff(n);
    const el=document.querySelector('.key[data-midi="'+n+'"]');if(el)el.classList.remove('pressed')}});
document.addEventListener('keydown',e=>{if(!e.ctrlKey)return;
  if(e.key==='z'&&calibStep===2){e.preventDefault();undoFp()}
  if(e.key==='y'&&calibStep===2){e.preventDefault();redoFp()}});

// --- SVG FLUTE ---
function buildFlute(cfg,svgId,showNums){
  const svg=$(svgId);if(!svg||!cfg)return;
  const nf=cfg.num_fingers||6,fingers=cfg.fingers||[];
  const sp=50,sx=80,r=14;
  const topHoles=[],botHoles=[];
  for(let i=0;i<nf;i++){(fingers[i]&&fingers[i].th?botHoles:topHoles).push(i)}
  const posTop=[];
  for(let i=0;i<topHoles.length;i++){posTop.push(sx+i*sp)}
  const posBot=[];
  for(let i=0;i<botHoles.length;i++){posBot.push(sx+i*sp)}
  const allX=[...posTop,...posBot,sx+200];
  const tw=Math.max(...allX)+60;
  const h_top=35,h_bot=65,cy=50;
  svg.setAttribute('viewBox','0 0 '+tw+' 100');
  const g=svgId,ty=34,by=66,th=by-ty;
  let h='<defs><linearGradient id="wg_'+g+'" x1="0" y1="0" x2="0" y2="1">'+
    '<stop offset="0%" stop-color="#D4B044"/><stop offset="35%" stop-color="#C4A035"/>'+
    '<stop offset="70%" stop-color="#9B7A1C"/><stop offset="100%" stop-color="#6B4F10"/></linearGradient>'+
    '<linearGradient id="lp_'+g+'" x1="0" y1="0" x2="0" y2="1">'+
    '<stop offset="0%" stop-color="#E8CC60"/><stop offset="40%" stop-color="#D4B044"/>'+
    '<stop offset="100%" stop-color="#A8862A"/></linearGradient>'+
    '<linearGradient id="cr_'+g+'" x1="0" y1="0" x2="0" y2="1">'+
    '<stop offset="0%" stop-color="#B89530"/><stop offset="50%" stop-color="#8A6A18"/>'+
    '<stop offset="100%" stop-color="#5C4A0A"/></linearGradient>'+
    '<linearGradient id="eh_'+g+'" x1="0" y1="0" x2="0" y2="1">'+
    '<stop offset="0%" stop-color="#1A1008"/><stop offset="100%" stop-color="#0A0600"/></linearGradient></defs>';
  // Corps de la flute - vue de cote, bords droits
  h+='<rect x="14" y="'+ty+'" width="'+(tw-24)+'" height="'+th+'" rx="0" fill="url(#wg_'+g+')" stroke="#5C4A0A" stroke-width="1.5"/>';
  // Reflet horizontal haut du tube (effet cylindrique)
  h+='<rect x="14" y="'+ty+'" width="'+(tw-24)+'" height="6" rx="0" fill="#D4B044" opacity=".18"/>';
  // Couronne - bouchon en bout, angles droits, legerement plus haut
  h+='<rect x="4" y="'+(ty-3)+'" width="12" height="'+(th+6)+'" rx="1" fill="url(#cr_'+g+')" stroke="#5C4A0A" stroke-width="1.2"/>';
  // Rainure decorative de la couronne
  h+='<line x1="10" y1="'+(ty-2)+'" x2="10" y2="'+(by+2)+'" stroke="#5C4A0A" stroke-width=".6" opacity=".5"/>';
  // Bague de jonction tete/corps
  h+='<rect x="62" y="'+(ty-1)+'" width="4" height="'+(th+2)+'" rx="0" fill="#A8862A" stroke="#5C4A0A" stroke-width=".6" opacity=".7"/>';
  // Plaque de levre (lip plate) - bosse vue de cote, depasse au-dessus du tube
  h+='<path d="M28,'+ty+' Q28,'+(ty-14)+' 42,'+(ty-16)+' Q56,'+(ty-14)+' 56,'+ty+'" fill="url(#lp_'+g+')" stroke="#5C4A0A" stroke-width="1.2"/>';
  // Reflet sur la plaque de levre
  h+='<path d="M31,'+(ty-2)+' Q31,'+(ty-11)+' 42,'+(ty-13)+' Q53,'+(ty-11)+' 53,'+(ty-2)+'" fill="none" stroke="#EDD580" stroke-width=".7" opacity=".3"/>';
  // Trou d\'embouchure (blow hole) - fente sombre dans la plaque
  h+='<rect x="36" y="'+(ty-10)+'" width="12" height="8" rx="3" fill="url(#eh_'+g+')" stroke="#3D2A08" stroke-width=".8"/>';
  // Biseau - arete vive cote droit du trou (ou l\'air se fend)
  h+='<line x1="48" y1="'+(ty-9)+'" x2="48" y2="'+(ty-3)+'" stroke="#D4B044" stroke-width="1" opacity=".5"/>';
  // Ombre interieure du trou
  h+='<rect x="37" y="'+(ty-9)+'" width="10" height="6" rx="2" fill="none" stroke="#0A0600" stroke-width=".4" opacity=".3"/>';
  // Top holes
  topHoles.forEach((fi,i)=>{
    h+='<circle id="fh_'+svgId+'_'+fi+'" cx="'+posTop[i]+'" cy="'+h_top+'" r="'+r+'" class="flute-hole closed"/>';
    if(showNums)h+='<text x="'+posTop[i]+'" y="'+(h_top+4)+'" text-anchor="middle" class="flute-num">'+(fi+1)+'</text>'
  });
  // Bottom holes (thumb)
  botHoles.forEach((fi,i)=>{
    h+='<circle id="fh_'+svgId+'_'+fi+'" cx="'+posBot[i]+'" cy="'+h_bot+'" r="'+(r-2)+'" class="flute-hole closed thumb"/>';
    if(showNums)h+='<text x="'+posBot[i]+'" y="'+(h_bot+4)+'" text-anchor="middle" class="flute-num">'+(fi+1)+'</text>'
  });
  if(botHoles.length>0)h+='<text x="'+(posBot[0])+'" y="88" text-anchor="middle" class="flute-lbl">Pouce</text>';
  svg.innerHTML=h
}

function updateFluteForNote(midi){
  if(!CFG)return;const nd=CFG.notes.find(n=>n.midi===midi);
  for(let i=0;i<CFG.num_fingers;i++){const el=$('fh_fluteSvg_'+i);
    if(el)el.setAttribute('class','flute-hole '+(nd&&nd.fp[i]?'open':'closed')+(CFG.fingers[i]&&CFG.fingers[i].th?' thumb':''))}
  $('fluteNote').textContent=nd?mn(nd.midi):'-';$('fluteInfo').textContent=nd?'MIDI '+nd.midi:''
}

// --- MIDI UPLOAD ---
const dz=$('dropZone');
dz.addEventListener('dragover',e=>{e.preventDefault();dz.classList.add('hover')});
dz.addEventListener('dragleave',()=>dz.classList.remove('hover'));
dz.addEventListener('drop',e=>{e.preventDefault();dz.classList.remove('hover');
  if(e.dataTransfer.files.length)uploadMidiFile(e.dataTransfer.files[0])});
function uploadMidi(input){if(input.files.length)uploadMidiFile(input.files[0])}
function uploadMidiFile(file){
  const fd=new FormData();fd.append('file',file);
  const ub=$('uploadBar'),uf=$('uploadFill');ub.style.display='block';uf.style.width='0%';
  const xhr=new XMLHttpRequest();
  xhr.upload.onprogress=e=>{if(e.lengthComputable)uf.style.width=(e.loaded/e.total*100)+'%'};
  xhr.onload=()=>{uf.style.width='100%';setTimeout(()=>ub.style.display='none',1000);
    try{const d=JSON.parse(xhr.responseText);if(d.ok){showToast('Upload OK: '+d.events+' evt','success');addLog('Upload OK')}
    else{showToast('Erreur: '+(d.msg||'echec'),'error')}}catch(e){showToast('Erreur upload','error')}};
  xhr.onerror=()=>{ub.style.display='none';showToast('Erreur upload reseau','error')};
  xhr.open('POST','/api/midi');xhr.send(fd)
}

// --- CALIBRATION ---
function buildCalibUI(){if(!CFG)return;buildFlute(CFG,'calFluteSvg',true);buildFingerCards();goStep(calibStep)}

function goStep(s){
  calibStep=s;
  ['step1','step2','step3'].forEach((id,i)=>{const el=$(id);el.style.display=(i+1===s)?'':'none';
    if(i+1===s){el.classList.add('fade-in')}else{el.classList.remove('fade-in')}});
  document.querySelectorAll('.step-dot').forEach((d,i)=>{d.className='step-dot'+(i+1===s?' active':i+1<s?' done':' locked')});
  if(s===2){buildPresetSelect();
    const iv=$('instrumentSelect');if(iv&&iv.value){$('presetSelect').value=iv.value;
      // Auto-apply si les notes ne sont pas encore remplies ou viennent d'un autre preset
      if(!CFG.notes.length||CFG._lastInst!==iv.value){applyPreset(iv.value);CFG._lastInst=iv.value}}
    buildFingeringRows();fpHistory=[];fpFuture=[];updUndoUI()}
  if(s===3)buildAirflowRows()
}

function changeFingers(delta){
  if(!CFG)return;let nf=CFG.num_fingers+delta;
  if(nf<1)nf=1;if(nf>MAX_FINGERS)nf=MAX_FINGERS;CFG.num_fingers=nf;
  // Add defaults for new fingers
  while(CFG.fingers.length<nf)CFG.fingers.push({ch:CFG.fingers.length,a:90,d:1,th:0});
  $('numFingersDisp').textContent=nf;buildFingerCards();buildFlute(CFG,'calFluteSvg',true);markDirty()
}

function buildFingerCards(){
  const c=$('fingerCards');c.innerHTML='';if(!CFG)return;
  $('numFingersDisp').textContent=CFG.num_fingers;
  $('angleOpen').value=CFG.angle_open||30;$('aoVal').textContent=(CFG.angle_open||30)+'deg';
  // PCA dropdown
  const sel=$('airPca');sel.innerHTML='';
  for(let i=0;i<16;i++){const o=document.createElement('option');o.value=i;o.textContent='PCA '+i;sel.appendChild(o)}
  sel.value=CFG.air_pca||10;
  // Finger cards
  for(let i=0;i<CFG.num_fingers;i++){
    const f=CFG.fingers[i]||{ch:i,a:90,d:1,th:0};
    const d=document.createElement('div');d.className='cal-card';
    let html='<div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:6px">'+
      '<h4 style="margin:0">Doigt '+(i+1)+'</h4>'+
      '<div style="display:flex;align-items:center;gap:8px">'+
        '<span style="font-size:.75em;color:#888">Pin PCA</span>'+
        '<select id="fch'+i+'" style="max-width:70px" onchange="CFG.fingers['+i+'].ch=parseInt(this.value);checkPca();markDirty()">'+
          Array.from({length:16},(_,j)=>'<option value="'+j+'"'+(j===f.ch?' selected':'')+'>'+j+'</option>').join('')+'</select>'+
        '<select id="fd'+i+'" style="max-width:60px" onchange="CFG.fingers['+i+'].d=parseInt(this.value);markDirty()">'+
          '<option value="1"'+(f.d===1?' selected':'')+'>+1</option><option value="-1"'+(f.d===-1?' selected':'')+'>-1</option></select>'+
      '</div></div>';
    if(i===0) html+='<div class="cfg-row"><label>Pouce (arriere)</label><input type="checkbox" id="fth'+i+'"'+(f.th?' checked':'')+
      ' style="width:auto;flex:0" onchange="CFG.fingers['+i+'].th=this.checked?1:0;buildFlute(CFG,\'calFluteSvg\',true);markDirty()"></div>';
    html+='<div style="margin:6px 0"><div style="display:flex;justify-content:space-between;font-size:.75em;color:#888;margin-bottom:2px">'+
      '<span>Angle ferme</span><span id="fav'+i+'">'+f.a+'&deg;</span></div>'+
      '<input type="range" min="0" max="180" value="'+f.a+'" id="fa'+i+'" style="width:100%"'+
        ' oninput="CFG.fingers['+i+'].a=parseInt(this.value);$(\'fav'+i+'\').textContent=this.value+\'&deg;\';testFinger('+i+',parseInt(this.value))"></div>'+
      '<div class="btn-row"><button class="btn btn-s" onclick="testPulse(this);testFinger('+i+',CFG.fingers['+i+'].a)">Fermer</button>'+
        '<button class="btn btn-s" onclick="testPulse(this);testFinger('+i+',CFG.fingers['+i+'].a+(CFG.angle_open||30)*CFG.fingers['+i+'].d)">Ouvrir</button></div>'+
      '<div class="pca-warn"></div>';
    d.innerHTML=html;c.appendChild(d)
  }
  checkPca();buildInstrumentSelect()
}

function buildInstrumentSelect(){
  const s=$('instrumentSelect');if(!s)return;
  s.innerHTML='<option value="">-- Personnalis\u00e9 --</option>';
  const groups={};PR.forEach(p=>{(groups[p.h]=groups[p.h]||[]).push(p)});
  Object.keys(groups).sort((a,b)=>a-b).forEach(h=>{
    const og=document.createElement('optgroup');og.label=h+' trous';
    groups[h].forEach(p=>{const o=document.createElement('option');o.value=p.id;
      o.textContent=p.n+(p.th>=0?' (pouce)':'');og.appendChild(o)});
    s.appendChild(og)})
}

function selectInstrument(val){
  if(!val||!CFG)return;
  const p=PR.find(x=>x.id===val);if(!p)return;
  // Step 1 = config physique seulement (trous + pouce), pas les notes
  CFG.num_fingers=p.h;
  while(CFG.fingers.length<p.h)CFG.fingers.push({ch:CFG.fingers.length,a:90,d:1,th:0});
  CFG.fingers.forEach(f=>f.th=0);
  if(p.th>=0&&CFG.fingers[p.th])CFG.fingers[p.th].th=1;
  // Rebuild UI physique
  buildFingerCards();buildFlute(CFG,'calFluteSvg',true);markDirty();
  showToast(p.n+' - '+p.h+' trous'+(p.th>=0?' (pouce)':''),'success')
}

function testFinger(i,a){wsSend({t:'test_finger',i:i,a:parseInt(a)});
  // Update flute SVG
  const el=$('fh_calFluteSvg_'+i);if(el){const closed=CFG.fingers[i].a;const open=Math.abs(a-closed)>(CFG.angle_open||30)/2;
    el.setAttribute('class','flute-hole '+(open?'open':'closed')+(CFG.fingers[i].th?' thumb':''))}}

function saveStep1(){
  if(!CFG)return;btnLoad('btnSaveStep1',true);
  CFG.angle_open=parseInt($('angleOpen').value);CFG.air_pca=parseInt($('airPca').value);
  for(let i=0;i<CFG.num_fingers;i++){
    CFG.fingers[i].ch=parseInt($('fch'+i).value);
    CFG.fingers[i].a=parseInt($('fa'+i).value);
    CFG.fingers[i].d=parseInt($('fd'+i).value);
    const thEl=$('fth'+i);CFG.fingers[i].th=thEl?thEl.checked?1:0:0
  }
  const body={num_fingers:CFG.num_fingers,air_pca:CFG.air_pca,angle_open:CFG.angle_open,fingers:CFG.fingers.slice(0,CFG.num_fingers)};
  fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)})
    .then(r=>r.json()).then(d=>{btnLoad('btnSaveStep1',false);if(d.ok){showToast('Doigts sauvegardes','success');markClean();goStep(2)}else showToast('Erreur sauvegarde','error')})
    .catch(e=>{btnLoad('btnSaveStep1',false);showToast('Erreur: '+e,'error')})
}

// --- STEP 2: FINGERINGS ---
function buildFingeringRows(){
  const c=$('fingeringRows');c.innerHTML='';if(!CFG)return;let lastOct=-1;
  CFG.notes.forEach((n,ni)=>{
    const oct=Math.floor(n.midi/12)-1;if(oct!==lastOct){lastOct=oct;
      const sep=document.createElement('div');sep.className='fg-octave';sep.textContent='Octave '+oct;c.appendChild(sep)}
    const d=document.createElement('div');d.className='fg-row fade-in fade-delay-'+(Math.min(4,(ni%4)+1));
    let dots='';for(let f=0;f<CFG.num_fingers;f++){
      const isThumb=CFG.fingers[f]&&CFG.fingers[f].th;
      dots+='<div class="fg-dot '+(n.fp[f]?'open':'closed')+(isThumb?' thumb':'')+'" data-ni="'+ni+'" data-fi="'+f+'" onclick="toggleFP('+ni+','+f+',this)"></div>'
    }
    d.innerHTML='<input type="number" class="fg-midi" style="width:48px" value="'+n.midi+'" min="0" max="127" onchange="fpSnap();CFG.notes['+ni+'].midi=parseInt(this.value);markDirty()">'+
      '<span class="fg-note">'+mn(n.midi)+'</span>'+
      '<div class="fg-dots">'+dots+'</div>'+
      '<button class="btn btn-s" style="padding:4px 8px;font-size:.75em" onclick="testPulse(this);wsSend({t:\'test_note\',n:'+n.midi+'})">Test</button>';
    c.appendChild(d)
  })
}

function toggleFP(ni,fi,el){
  fpSnap();CFG.notes[ni].fp[fi]=CFG.notes[ni].fp[fi]?0:1;
  el.className='fg-dot '+(CFG.notes[ni].fp[fi]?'open':'closed')+(CFG.fingers[fi]&&CFG.fingers[fi].th?' thumb':'');markDirty()
}

function addNote(){
  if(!CFG)return;fpSnap();const last=CFG.notes.length?CFG.notes[CFG.notes.length-1]:null;
  const midi=last?last.midi+2:84;const fp=new Array(CFG.num_fingers).fill(0);
  CFG.notes.push({midi:midi,fp:fp,amn:20,amx:75});CFG.num_notes=CFG.notes.length;buildFingeringRows();markDirty()
}
function removeLastNote(){if(!CFG||!CFG.notes.length)return;fpSnap();CFG.notes.pop();CFG.num_notes=CFG.notes.length;buildFingeringRows();markDirty()}

function buildPresetSelect(){
  const s=$('presetSelect');if(!s||!CFG)return;
  s.innerHTML='<option value="">-- Personnalis\u00e9 --</option>';
  const nf=CFG.num_fingers;
  // Presets compatibles (meme nombre de trous)
  const compat=PR.filter(p=>p.h===nf);
  // Presets autres (nb trous different)
  const other=PR.filter(p=>p.h!==nf);
  if(compat.length){
    const og=document.createElement('optgroup');og.label='Compatible ('+nf+' trous)';
    compat.forEach(p=>{const o=document.createElement('option');o.value=p.id;
      o.textContent=p.n+' - '+p.d.length+' notes ('+mn(p.d[0][0])+'\u2192'+mn(p.d[p.d.length-1][0])+')';og.appendChild(o)});
    s.appendChild(og)}
  if(other.length){
    const og2=document.createElement('optgroup');og2.label='Autres instruments';
    other.forEach(p=>{const o=document.createElement('option');o.value=p.id;
      o.textContent=p.n+' ('+p.h+' trous, '+p.d.length+' notes)';o.style.opacity='.6';og2.appendChild(o)});
    s.appendChild(og2)}
  updPresetInfo()
}

function updPresetInfo(){
  let el=$('presetInfo');
  if(!el){const p=$('presetSelect');if(!p)return;el=document.createElement('div');el.id='presetInfo';
    el.style.cssText='font-size:.8em;color:#aaa;margin-top:4px;padding:6px 10px;background:rgba(255,255,255,.04);border-radius:6px';
    p.parentNode.appendChild(el)}
  const val=$('presetSelect').value;
  if(!val){el.innerHTML=CFG&&CFG.notes.length?'<b>Personnalis\u00e9</b> - '+CFG.notes.length+' notes ('+mn(CFG.notes[0].midi)+'\u2192'+mn(CFG.notes[CFG.notes.length-1].midi)+')':'Selectionnez un preset ou ajoutez des notes manuellement';return}
  const p=PR.find(x=>x.id===val);if(!p){el.innerHTML='';return}
  const lo=mn(p.d[0][0]),hi=mn(p.d[p.d.length-1][0]);
  const warn=p.h!==CFG.num_fingers?' <span style="color:#e94560">Attention: '+p.h+' trous \u2260 '+CFG.num_fingers+' configures</span>':'';
  el.innerHTML='<b>'+esc(p.n)+'</b> - '+p.d.length+' notes, de '+lo+' a '+hi+(p.th>=0?' (pouce doigt '+(p.th+1)+')':'')+warn
}

function applyPreset(val){
  if(!val||!CFG)return;
  const p=PR.find(x=>x.id===val);if(!p)return;
  // Ajuster num_fingers si different (avec avertissement)
  if(CFG.num_fingers!==p.h){
    CFG.num_fingers=p.h;
    while(CFG.fingers.length<p.h)CFG.fingers.push({ch:CFG.fingers.length,a:90,d:1,th:0});
    showToast('Nombre de doigts ajust\u00e9 \u00e0 '+p.h,'info')}
  // Set thumb
  CFG.fingers.forEach(f=>f.th=0);
  if(p.th>=0&&CFG.fingers[p.th])CFG.fingers[p.th].th=1;
  // Build notes from preset data
  CFG.notes=p.d.map(n=>({midi:n[0],fp:[...n[1]],amn:n[2],amx:n[3]}));
  CFG.notes.forEach(n=>{while(n.fp.length<CFG.num_fingers)n.fp.push(0)});
  CFG.num_notes=CFG.notes.length;
  fpSnap();buildFingeringRows();buildFlute(CFG,'calFluteSvg',true);updPresetInfo();markDirty()
}

function saveStep2(){
  if(!CFG)return;btnLoad('btnSaveStep2',true);
  const body={notes:CFG.notes.map(n=>({midi:n.midi,fp:n.fp.slice(0,CFG.num_fingers),amn:n.amn,amx:n.amx}))};
  fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)})
    .then(r=>r.json()).then(d=>{btnLoad('btnSaveStep2',false);if(d.ok){showToast('Doigtes sauvegardes','success');markClean();fpHistory=[];fpFuture=[];updUndoUI();goStep(3);buildKeyboard()}else showToast('Erreur sauvegarde','error')})
    .catch(e=>{btnLoad('btnSaveStep2',false);showToast('Erreur: '+e,'error')})
}

// --- STEP 3: AIRFLOW ---
function buildAirflowRows(){
  const c=$('airflowRows');c.innerHTML='';if(!CFG)return;
  CFG.notes.forEach((n,ni)=>{
    let dots='';for(let f=0;f<CFG.num_fingers;f++)dots+='<span class="kf '+(n.fp[f]?'o':'c')+'"></span>';
    const d=document.createElement('div');d.className='air-card fade-in fade-delay-'+(Math.min(4,(ni%4)+1));
    d.innerHTML='<span class="air-note">'+mn(n.midi)+'</span>'+
      '<span class="kf-row" style="gap:2px">'+dots+'</span>'+
      '<div class="air-sliders">'+
        '<div class="air-vals"><span>Min: <b id="amn'+ni+'">'+n.amn+'</b>%</span><span>Max: <b id="amx'+ni+'">'+n.amx+'</b>%</span></div>'+
        '<div class="dual-range"><div class="dual-range-track"></div><div class="dual-range-fill" id="drf'+ni+'"></div>'+
        '<input type="range" min="0" max="100" value="'+n.amn+'" oninput="CFG.notes['+ni+'].amn=parseInt(this.value);$(\'amn'+ni+'\').textContent=this.value;updDualFill('+ni+');markDirty()">'+
        '<input type="range" min="0" max="100" value="'+n.amx+'" oninput="CFG.notes['+ni+'].amx=parseInt(this.value);$(\'amx'+ni+'\').textContent=this.value;updDualFill('+ni+');markDirty()"></div>'+
      '</div>'+
      '<button class="btn btn-s" style="padding:4px 8px;font-size:.75em" onclick="testPulse(this);testCalNote('+n.midi+')">Test</button>';
    c.appendChild(d);updDualFill(ni)
  })
}

function testCalNote(midi){wsSend({t:'test_note',n:midi});wsSend({t:'test_sol',o:1});
  setTimeout(()=>wsSend({t:'test_sol',o:0}),TEST_SOL_MS)}

function startAutoCal(){autoCalRunning=true;$('btnAcalStart').style.display='none';$('btnAcalStop').style.display='';
  $('acalProgress').style.display='block';$('acalResults').style.display='none';wsSend({t:'auto_cal',mode:'air'})}
function stopAutoCal(){autoCalRunning=false;$('btnAcalStart').style.display='';$('btnAcalStop').style.display='none';
  wsSend({t:'auto_cal',mode:'stop'})}

function saveStep3(){
  if(!CFG)return;btnLoad('btnSaveStep3',true);
  const body={notes_air:CFG.notes.map(n=>({amn:n.amn,amx:n.amx}))};
  fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)})
    .then(r=>r.json()).then(d=>{btnLoad('btnSaveStep3',false);if(d.ok){showToast('Calibration terminee !','success');markClean();buildKeyboard()}else showToast('Erreur sauvegarde','error')})
    .catch(e=>{btnLoad('btnSaveStep3',false);showToast('Erreur: '+e,'error')})
}

// --- SETTINGS ---
function fillSettings(){
  if(!CFG)return;
  $('cfgDevice').value=CFG.device||'';
  const sel=$('cfgMidiCh');sel.innerHTML='<option value="0">Omni (tous)</option>';
  for(let i=1;i<=16;i++){const o=document.createElement('option');o.value=i;o.textContent='Canal '+i;sel.appendChild(o)}
  sel.value=CFG.midi_ch||0;
  $('cfgDelay').value=CFG.servo_delay;$('cfgValveInt').value=CFG.valve_interval;$('cfgMinNote').value=CFG.min_note_dur;
  $('cfgAirOff').value=CFG.air_off;$('cfgAirMin').value=CFG.air_min;$('cfgAirMax').value=CFG.air_max;
  $('cfgVibF').value=CFG.vib_freq;$('cfgVibA').value=CFG.vib_amp;
  $('cfgCCVol').value=CFG.cc_vol!=null?CFG.cc_vol:127;$('cfgCCExpr').value=CFG.cc_expr!=null?CFG.cc_expr:127;
  $('cfgCCMod').value=CFG.cc_mod!=null?CFG.cc_mod:0;$('cfgCCBreath').value=CFG.cc_breath!=null?CFG.cc_breath:127;
  $('cfgCCBright').value=CFG.cc_bright!=null?CFG.cc_bright:64;
  $('cfgCC2On').checked=CFG.cc2_on;$('cfgCC2Thr').value=CFG.cc2_thr;$('cfgCC2Curve').value=CFG.cc2_curve;$('cfgCC2To').value=CFG.cc2_timeout;
  $('cfgSolAct').value=CFG.sol_act;$('cfgSolHold').value=CFG.sol_hold;$('cfgSolTime').value=CFG.sol_time;
  $('cfgUnpower').value=CFG.time_unpower;
  $('wifiSsid').value=CFG.wifi_ssid||'';
  // WiFi status
  fetch('/api/wifi/status').then(r=>r.json()).then(d=>{
    $('wifiState').textContent=d.ap?'AP: '+d.ip:'STA: '+(d.ip||'deconnecte')+(d.rssi?' ('+d.rssi+' dBm)':'')
  }).catch(()=>{})
}

function saveSettings(){
  btnLoad('btnSaveSettings',true);
  const body={device:$('cfgDevice').value,midi_ch:parseInt($('cfgMidiCh').value),
    servo_delay:parseInt($('cfgDelay').value),valve_interval:parseInt($('cfgValveInt').value),
    min_note_dur:parseInt($('cfgMinNote').value),
    air_off:parseInt($('cfgAirOff').value),air_min:parseInt($('cfgAirMin').value),air_max:parseInt($('cfgAirMax').value),
    vib_freq:parseFloat($('cfgVibF').value),vib_amp:parseFloat($('cfgVibA').value),
    cc_vol:parseInt($('cfgCCVol').value),cc_expr:parseInt($('cfgCCExpr').value),
    cc_mod:parseInt($('cfgCCMod').value),cc_breath:parseInt($('cfgCCBreath').value),cc_bright:parseInt($('cfgCCBright').value),
    cc2_on:$('cfgCC2On').checked,cc2_thr:parseInt($('cfgCC2Thr').value),
    cc2_curve:parseFloat($('cfgCC2Curve').value),cc2_timeout:parseInt($('cfgCC2To').value),
    sol_act:parseInt($('cfgSolAct').value),sol_hold:parseInt($('cfgSolHold').value),sol_time:parseInt($('cfgSolTime').value),
    time_unpower:parseInt($('cfgUnpower').value)};
  fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)})
    .then(r=>r.json()).then(d=>{btnLoad('btnSaveSettings',false);
      if(d.ok){showToast('Parametres sauvegardes','success');markClean();loadConfig()}
      else showToast('Erreur sauvegarde','error');
      $('settingsMsg').textContent=d.ok?'Sauvegarde OK':'Erreur';$('settingsMsg').style.color=d.ok?'#4ecca3':'#e94560'})
    .catch(e=>{btnLoad('btnSaveSettings',false);showToast('Erreur: '+e,'error');$('settingsMsg').textContent='Erreur: '+e;$('settingsMsg').style.color='#e94560'})
}

function resetConfig(){if(!confirm('Remettre tous les parametres par defaut ?'))return;
  fetch('/api/config/reset',{method:'POST'}).then(r=>r.json()).then(d=>{if(d.ok){addLog('Reset OK');loadConfig();fillSettings()}})
    .catch(e=>addLog('Erreur: '+e))}

// --- WIFI ---
function startWifiScan(){$('scanStatus').textContent='Scan...';
  fetch('/api/wifi/scan').then(()=>{setTimeout(checkScan,3000)}).catch(e=>{$('scanStatus').textContent='Erreur'})}
function checkScan(){fetch('/api/wifi/results').then(r=>r.json()).then(d=>{
    if(!d.done){setTimeout(checkScan,2000);return}
    $('scanStatus').textContent='';const c=$('wifiList');c.innerHTML='';
    if(d.networks)d.networks.forEach(n=>{
      const el=document.createElement('div');el.className='wifi-item';
      el.innerHTML='<span>'+esc(n.ssid)+'</span><span style="color:#888">'+esc(n.rssi)+' dBm</span>';
      el.onclick=()=>{$('wifiSsid').value=n.ssid};c.appendChild(el)})
  }).catch(()=>{$('scanStatus').textContent='Erreur'})}

function connectWifi(){const ssid=$('wifiSsid').value,pass=$('wifiPass').value;
  if(!ssid){$('wifiMsg').textContent='SSID requis';return}
  $('wifiMsg').textContent='Connexion...';
  fetch('/api/wifi/connect',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({ssid:ssid,pass:pass})})
    .then(r=>r.json()).then(d=>{$('wifiMsg').textContent=d.msg||'OK'})
    .catch(e=>{$('wifiMsg').textContent='Erreur: '+e})}

// --- INIT ---
window.addEventListener('load',()=>{$('velVal').textContent=WEB_DEF_VEL;$('velSlider').value=WEB_DEF_VEL;wsConnect();loadConfig()});
window.addEventListener('beforeunload',e=>{if(dirty){e.preventDefault();e.returnValue=''}});
</script>
</body>
</html>
)rawliteral";

#endif
