#include "webinterface.h"

#include <Arduino.h>
#include <WebServer.h>

#define WEBINTERFACE_PORT 80

static WebServer server(WEBINTERFACE_PORT);

/* -------------------------------------------------------------------
   Pointers to shared state owned by world-clock.ino
   ------------------------------------------------------------------- */
static ClockTime *s_clockTime   = nullptr;
static LocalTime *s_iceland     = nullptr;
static LocalTime *s_houston     = nullptr;
static LocalTime *s_bangkok     = nullptr;
static uint8_t   *s_alarmHour   = nullptr;
static uint8_t   *s_alarmMinute = nullptr;
static uint8_t   *s_brightness  = nullptr;
static Display   *s_display1    = nullptr;
static Display   *s_display2    = nullptr;
static Display   *s_display3    = nullptr;

/* -------------------------------------------------------------------
   HTML management page — stored in read-only flash (.rodata)
   ------------------------------------------------------------------- */
static const char HTML_PAGE[] = R"rawhtml(<!DOCTYPE html>
<html lang="en"><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>World Clock</title><style>
body{font-family:sans-serif;max-width:480px;margin:2rem auto;padding:0 1rem;background:#f5f5f5}
h1{text-align:center}
.card{background:#fff;border-radius:8px;padding:1rem;margin:1rem 0;box-shadow:0 1px 3px rgba(0,0,0,.15)}
.clocks{display:flex;justify-content:space-around}
.clock{text-align:center}
.city{font-size:.75rem;color:#888;text-transform:uppercase;letter-spacing:.05em}
.time{font-size:2.2rem;font-weight:700;font-family:monospace}
h2{margin:.25rem 0 .75rem;font-size:.85rem;color:#555;text-transform:uppercase;letter-spacing:.05em}
.row{display:flex;align-items:center;gap:.5rem;flex-wrap:wrap}
.row label{font-size:.85rem;color:#555;min-width:50px}
input[type=number]{width:52px;padding:3px 5px;border:1px solid #ccc;border-radius:4px;text-align:center}
input[type=range]{flex:1;min-width:100px}
button{padding:5px 14px;background:#0070c0;color:#fff;border:none;border-radius:4px;cursor:pointer}
button:hover{background:#005fa3}
#msg{font-size:.8rem}
</style></head><body>
<h1>World Clock</h1>
<div class="card"><div class="clocks">
  <div class="clock"><div class="city">Houston</div><div class="time" id="ht">--:--</div></div>
  <div class="clock"><div class="city">Bangkok</div><div class="time" id="bt">--:--</div></div>
  <div class="clock"><div class="city">Iceland</div><div class="time" id="it">--:--</div></div>
</div></div>
<div class="card">
  <h2>Alarm</h2>
  <div class="row">
    <label>Hour</label><input type="number" id="ah" min="0" max="23" value="0">
    <label>Minute</label><input type="number" id="am" min="0" max="59" value="0">
    <button onclick="setAlarm()">Set</button>
    <span id="msg"></span>
  </div>
</div>
<div class="card">
  <h2>Brightness</h2>
  <div class="row">
    <input type="range" id="br" min="0" max="7" step="1" value="7">
    <span id="bv" style="min-width:1rem;text-align:center">7</span>
    <button onclick="setBrightness()">Set</button>
  </div>
</div>
<script>
function p(v){return String(v).padStart(2,'0');}
function flash(ok){var e=document.getElementById('msg');e.style.color=ok?'green':'red';e.textContent=ok?'Saved':'Error';setTimeout(function(){e.textContent='';},2000);}
function update(){
  fetch('/status').then(function(r){return r.json();}).then(function(d){
    var ae=document.activeElement.id;
    document.getElementById('ht').textContent=d.houston.available?p(d.houston.hour)+':'+p(d.houston.minute):'--:--';
    document.getElementById('bt').textContent=d.bangkok.available?p(d.bangkok.hour)+':'+p(d.bangkok.minute):'--:--';
    document.getElementById('it').textContent=d.iceland.available?p(d.iceland.hour)+':'+p(d.iceland.minute):'--:--';
    if(ae!=='ah')document.getElementById('ah').value=d.alarm.hour;
    if(ae!=='am')document.getElementById('am').value=d.alarm.minute;
    if(ae!=='br'){document.getElementById('br').value=d.brightness;document.getElementById('bv').textContent=d.brightness;}
  }).catch(function(){});}
document.getElementById('br').oninput=function(){document.getElementById('bv').textContent=this.value;};
function setAlarm(){
  var h=parseInt(document.getElementById('ah').value,10);
  var m=parseInt(document.getElementById('am').value,10);
  if(isNaN(h)||isNaN(m)||h<0||h>23||m<0||m>59){flash(false);return;}
  fetch('/set/alarm',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'hour='+h+'&minute='+m})
    .then(function(r){flash(r.ok);}).catch(function(){flash(false);});}
function setBrightness(){
  var v=parseInt(document.getElementById('br').value,10);
  if(isNaN(v)||v<0||v>7){flash(false);return;}
  fetch('/set/brightness',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'value='+v})
    .then(function(r){flash(r.ok);}).catch(function(){flash(false);});}
update();setInterval(update,1000);
</script></body></html>)rawhtml";

/* -------------------------------------------------------------------
   Route handlers
   ------------------------------------------------------------------- */

/* Serves the single-page HTML management interface. */
static void handleRoot() {
  server.send(200, "text/html", HTML_PAGE);
}

/* Returns a JSON snapshot of current times, alarm, and brightness.
   The browser polls this endpoint every second for live updates. */
static void handleStatus() {
  const uint8_t hh = s_houston->isAvailable ? s_houston->hour   : 0;
  const uint8_t hm = s_houston->isAvailable ? s_houston->minute : 0;
  const uint8_t bh = s_bangkok->isAvailable ? s_bangkok->hour   : 0;
  const uint8_t bm = s_bangkok->isAvailable ? s_bangkok->minute : 0;
  const uint8_t ih = s_iceland->isAvailable ? s_iceland->hour   : 0;
  const uint8_t im = s_iceland->isAvailable ? s_iceland->minute : 0;

  char buf[300];
  snprintf(buf, sizeof(buf),
    "{\"houston\":{\"hour\":%u,\"minute\":%u,\"available\":%s},"
    "\"bangkok\":{\"hour\":%u,\"minute\":%u,\"available\":%s},"
    "\"iceland\":{\"hour\":%u,\"minute\":%u,\"available\":%s},"
    "\"alarm\":{\"hour\":%u,\"minute\":%u},"
    "\"brightness\":%u}",
    hh, hm, s_houston->isAvailable ? "true" : "false",
    bh, bm, s_bangkok->isAvailable ? "true" : "false",
    ih, im, s_iceland->isAvailable ? "true" : "false",
    static_cast<unsigned>(*s_alarmHour),
    static_cast<unsigned>(*s_alarmMinute),
    static_cast<unsigned>(*s_brightness));

  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", buf);
}

/* Accepts hour and minute from a form POST and validates them before
   updating the shared alarm state. Responds 204 on success, 400 on
   invalid input. */
static void handleSetAlarm() {
  if (!server.hasArg("hour") || !server.hasArg("minute")) {
    server.send(400, "text/plain", "Bad Request");
    return;
  }

  const int h = server.arg("hour").toInt();
  const int m = server.arg("minute").toInt();

  if (h < 0 || h > 23 || m < 0 || m > 59) {
    server.send(400, "text/plain", "Bad Request");
    return;
  }

  *s_alarmHour   = static_cast<uint8_t>(h);
  *s_alarmMinute = static_cast<uint8_t>(m);
  server.send(204, "text/plain", "");
}

/* Accepts a brightness value (0-7) from a form POST, clamps it, applies
   it immediately to all three displays, and updates shared state.
   Responds 204 on success, 400 on invalid input. */
static void handleSetBrightness() {
  if (!server.hasArg("value")) {
    server.send(400, "text/plain", "Bad Request");
    return;
  }

  const int v = server.arg("value").toInt();

  if (v < 0 || v > 7) {
    server.send(400, "text/plain", "Bad Request");
    return;
  }

  *s_brightness = static_cast<uint8_t>(v);
  s_display1->setBrightness(*s_brightness);
  s_display2->setBrightness(*s_brightness);
  s_display3->setBrightness(*s_brightness);
  server.send(204, "text/plain", "");
}

static void handleNotFound() {
  server.send(404, "text/plain", "Not Found");
}

/* -------------------------------------------------------------------
   Public API
   ------------------------------------------------------------------- */

void webinterface_initiate(
    ClockTime *clockTime,
    LocalTime *icelandTime,
    LocalTime *houstonTime,
    LocalTime *bangkokTime,
    uint8_t   *alarmHour,
    uint8_t   *alarmMinute,
    uint8_t   *brightness,
    Display   *display1,
    Display   *display2,
    Display   *display3)
{
  s_clockTime   = clockTime;
  s_iceland     = icelandTime;
  s_houston     = houstonTime;
  s_bangkok     = bangkokTime;
  s_alarmHour   = alarmHour;
  s_alarmMinute = alarmMinute;
  s_brightness  = brightness;
  s_display1    = display1;
  s_display2    = display2;
  s_display3    = display3;

  server.on("/",               HTTP_GET,  handleRoot);
  server.on("/status",         HTTP_GET,  handleStatus);
  server.on("/set/alarm",      HTTP_POST, handleSetAlarm);
  server.on("/set/brightness", HTTP_POST, handleSetBrightness);
  server.onNotFound(handleNotFound);

  server.begin();
}

void webinterface_handle() {
  server.handleClient();
}
