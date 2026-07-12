#include <SPI.h>
#include <MFRC522.h>
#include <TFT_eSPI.h>
#include <SD.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <time.h>

// ======================== PINS ========================
#define RFID_SS      5
#define RFID_RST    22
#define TFT_CS      15
#define TFT_DC       2
#define TFT_RST      4
#define SD_CS       10
#define BUZZER      25

// ======================== CONFIG ========================
#define WIFI_SSID       "CrewTrack"
#define WIFI_PASSWORD   "12345678"
#define MAX_EMPLOYEES   50
#define SCREEN_READY        0
#define SCREEN_SUCCESS      1
#define SCREEN_DUPLICATE    2
#define SCREEN_UNKNOWN      3
#define SCREEN_TIMEOUT_MS   3000
#define DEBOUNCE_MS         2000
#define EMPLOYEES_FILE      "/employees.json"
#define ATTENDANCE_DIR      "/attendance"
#define NTP_SERVER          "pool.ntp.org"
#define GMT_OFFSET_SEC      10800
#define DST_OFFSET_SEC      0

// ======================== GLOBALS ========================
MFRC522 rfid(RFID_SS, RFID_RST);
TFT_eSPI tft = TFT_eSPI();

int currentScreen = SCREEN_READY;
unsigned long screenChangeTime = 0;
String currentDate = "";
String currentTime = "";

struct Employee {
    String uid;
    String name;
    float dailyWage;
    String phone;
    bool active;
};

Employee employees[MAX_EMPLOYEES];
int employeeCount = 0;
bool sdReady = false;
bool timeSet = false;
unsigned long lastScanTime = 0;

// ======================== RFID ========================
String rfidScan() {
    if (!rfid.PICC_IsNewCardPresent()) return "";
    if (!rfid.PICC_ReadCardSerial()) return "";

    String uid = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
        if (rfid.uid.uidByte[i] < 0x10) uid += "0";
        uid += String(rfid.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();
    rfid.PICC_HaltA();
    return uid;
}

// ======================== BUZZER ========================
void beep(int ms) {
    digitalWrite(BUZZER, HIGH);
    delay(ms);
    digitalWrite(BUZZER, LOW);
}

void beepDouble() {
    beep(50);
    delay(50);
    beep(50);
}

// ======================== TIME ========================
String getDate() {
    struct tm t;
    if (!getLocalTime(&t)) return "2026_07_12";
    char buf[16];
    sprintf(buf, "%d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
    return String(buf);
}

String getTimeStr() {
    struct tm t;
    if (!getLocalTime(&t)) return "??:??";
    char buf[8];
    sprintf(buf, "%02d:%02d", t.tm_hour, t.tm_min);
    return String(buf);
}

String getDisplayDate() {
    struct tm t;
    if (!getLocalTime(&t)) return "12 Jul 2026";
    const char *m[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    char buf[32];
    sprintf(buf, "%02d %s %d", t.tm_mday, m[t.tm_mon], t.tm_year + 1900);
    return String(buf);
}

// ======================== DISPLAY ========================
void drawHeader() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("CREWTRACK", 120, 10);
    tft.drawFastHLine(10, 35, 220, TFT_GREEN);
}

void showReady() {
    currentScreen = SCREEN_READY;
    drawHeader();
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.drawString("Ready", 120, 70);
    tft.setTextSize(1);
    tft.drawString("Scan Employee Card", 120, 120);
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString(getDisplayDate() + "  " + currentTime, 120, 170);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("WiFi: " + String(WIFI_SSID), 120, 200);
    tft.drawString("192.168.4.1", 120, 215);
}

void showSuccess(String name) {
    currentScreen = SCREEN_SUCCESS;
    screenChangeTime = millis();
    drawHeader();
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(3);
    tft.drawString("OK", 120, 60);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.drawString(name, 120, 110);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(1);
    tft.drawString("Attendance Saved", 120, 145);
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString(getDisplayDate() + "  " + currentTime, 120, 220);
    beep(150);
}

void showDuplicate(String name) {
    currentScreen = SCREEN_DUPLICATE;
    screenChangeTime = millis();
    drawHeader();
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextSize(2);
    tft.drawString("DUPLICATE", 120, 60);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.drawString(name, 120, 110);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextSize(1);
    tft.drawString("Already Registered Today", 120, 145);
    beepDouble();
}

void showUnknown(String uid) {
    currentScreen = SCREEN_UNKNOWN;
    screenChangeTime = millis();
    drawHeader();
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setTextSize(3);
    tft.drawString("???", 120, 60);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.drawString("Unknown Card", 120, 110);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString(uid, 120, 135);
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString("Register via Dashboard", 120, 170);
    beep(500);
}

void showError(String msg) {
    drawHeader();
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setTextSize(2);
    tft.drawString("ERROR", 120, 70);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(msg, 120, 120);
}

void showBoot(String msg) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("CREWTRACK", 120, 50);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(msg, 120, 100);
}

// ======================== STORAGE ========================
String readAll(String path) {
    if (!sdReady) return "";
    File f = SD.open(path, FILE_READ);
    if (!f) return "";
    String s = "";
    while (f.available()) s += (char)f.read();
    f.close();
    return s;
}

bool appendLine(String path, String data) {
    if (!sdReady) return false;
    File f = SD.open(path, FILE_APPEND);
    if (!f) return false;
    f.println(data);
    f.close();
    return true;
}

bool fileExists(String path) {
    if (!sdReady) return false;
    return SD.exists(path);
}

// ======================== EMPLOYEES ========================
void loadEmployees() {
    if (!sdReady) {
        employeeCount = 0;
        Serial.println("No SD - add employees via dashboard after SD is connected");
        return;
    }

    if (!fileExists(EMPLOYEES_FILE)) {
        employeeCount = 0;
        File f = SD.open(EMPLOYEES_FILE, FILE_WRITE);
        if (f) { f.println("[]"); f.close(); }
        return;
    }

    String data = readAll(EMPLOYEES_FILE);
    if (data.length() == 0) { employeeCount = 0; return; }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, data);
    if (err) { employeeCount = 0; return; }

    employeeCount = 0;
    JsonArray arr = doc.as<JsonArray>();
    for (JsonObject obj : arr) {
        if (employeeCount >= MAX_EMPLOYEES) break;
        employees[employeeCount].uid = obj["uid"].as<String>();
        employees[employeeCount].name = obj["name"].as<String>();
        employees[employeeCount].dailyWage = obj["wage"] | 50.0f;
        employees[employeeCount].phone = obj["phone"] | "";
        employees[employeeCount].active = true;
        employeeCount++;
    }
}

bool saveEmployees() {
    if (!sdReady) return false;
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();
    for (int i = 0; i < employeeCount; i++) {
        if (!employees[i].active) continue;
        JsonObject obj = arr.createNestedObject();
        obj["uid"] = employees[i].uid;
        obj["name"] = employees[i].name;
        obj["wage"] = employees[i].dailyWage;
        obj["phone"] = employees[i].phone;
    }
    String output;
    serializeJson(doc, output);
    SD.remove(EMPLOYEES_FILE);
    File f = SD.open(EMPLOYEES_FILE, FILE_WRITE);
    if (!f) return false;
    f.print(output);
    f.close();
    return true;
}

Employee* findByUID(String uid) {
    for (int i = 0; i < employeeCount; i++) {
        if (employees[i].uid == uid && employees[i].active) return &employees[i];
    }
    return nullptr;
}

bool addEmployee(String uid, String name, float wage, String phone) {
    if (employeeCount >= MAX_EMPLOYEES) return false;
    if (findByUID(uid) != nullptr) return false;
    employees[employeeCount].uid = uid;
    employees[employeeCount].name = name;
    employees[employeeCount].dailyWage = wage;
    employees[employeeCount].phone = phone;
    employees[employeeCount].active = true;
    employeeCount++;
    return saveEmployees();
}

bool removeEmployee(String uid) {
    for (int i = 0; i < employeeCount; i++) {
        if (employees[i].uid == uid) {
            for (int j = i; j < employeeCount - 1; j++) {
                employees[j] = employees[j + 1];
            }
            employeeCount--;
            return saveEmployees();
        }
    }
    return false;
}

bool updateEmployee(String uid, String name, float wage, String phone) {
    for (int i = 0; i < employeeCount; i++) {
        if (employees[i].uid == uid) {
            employees[i].name = name;
            employees[i].dailyWage = wage;
            employees[i].phone = phone;
            return saveEmployees();
        }
    }
    return false;
}

// ======================== ATTENDANCE ========================
String attPath(String date) {
    return "/attendance/att_" + date + ".csv";
}

bool isDuplicateScan(String uid, String date) {
    String data = readAll(attPath(date));
    if (data.length() == 0) return false;
    int idx = 0;
    while (idx < (int)data.length()) {
        int endIdx = data.indexOf('\n', idx);
        if (endIdx == -1) endIdx = data.length();
        String line = data.substring(idx, endIdx);
        idx = endIdx + 1;
        int c = line.indexOf(',');
        if (c > 0 && line.substring(0, c) == uid) return true;
    }
    return false;
}

bool logAttendance(String uid, String date, String time) {
    return appendLine(attPath(date), uid + "," + time);
}

int countToday(String date) {
    String data = readAll(attPath(date));
    if (data.length() == 0) return 0;
    int count = 0;
    for (unsigned int i = 0; i < data.length(); i++) {
        if (data[i] == '\n') count++;
    }
    return count;
}

String padNum(int n) { return n < 10 ? "0" + String(n) : String(n); }

int getDaysWorked(String uid, int month, int year) {
    int days = 0;
    for (int d = 1; d <= 31; d++) {
        String date = String(year) + "_" + padNum(month) + "_" + padNum(d);
        if (!fileExists(attPath(date))) continue;
        String data = readAll(attPath(date));
        int idx = 0;
        while (idx < (int)data.length()) {
            int endIdx = data.indexOf('\n', idx);
            if (endIdx == -1) endIdx = data.length();
            String line = data.substring(idx, endIdx);
            idx = endIdx + 1;
            int c = line.indexOf(',');
            if (c > 0 && line.substring(0, c) == uid) { days++; break; }
        }
    }
    return days;
}

// ======================== WEB DASHBOARD ========================
WebServer *server = NULL;
String lastScanName = "";
String lastScanTimeStr = "";

void handleRoot();
void handleAPIDashboard();
void handleAPIEmployees();
void handleAPIAddEmployee();
void handleAPIRemoveEmployee();
void handleAPIUpdateEmployee();
void handleAPIToday();
void handleAPIMonthly();
void handleAPIAttendance();
void handleAPIExportCSV();
void handleAPISetTime();
void handleAPIStatus();

const char PAGE_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>CrewTrack</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:'Segoe UI',sans-serif;background:#0f172a;color:#e2e8f0;min-height:100vh}
.hdr{background:linear-gradient(135deg,#059669,#10b981);padding:20px;text-align:center;position:sticky;top:0;z-index:100}
.hdr h1{font-size:1.8rem;color:#fff}
.hdr p{color:#d1fae5;font-size:.9rem}
.nav{display:flex;justify-content:center;gap:8px;padding:12px;background:#1e293b;flex-wrap:wrap}
.nav button{padding:10px 16px;border:none;border-radius:8px;cursor:pointer;font-size:.85rem;font-weight:600;background:#334155;color:#94a3b8;transition:.2s}
.nav button.active{background:#059669;color:#fff}
.nav button:hover{background:#475569}
.ct{max-width:900px;margin:0 auto;padding:16px}
.card{background:#1e293b;border-radius:12px;padding:20px;margin-bottom:16px;border:1px solid #334155}
.card h2{color:#10b981;margin-bottom:12px;font-size:1.2rem}
.sg{display:grid;grid-template-columns:repeat(auto-fit,minmax(140px,1fr));gap:12px;margin-bottom:16px}
.sb{background:#0f172a;border-radius:8px;padding:16px;text-align:center;border:1px solid #334155}
.sb .n{font-size:2rem;font-weight:700;color:#10b981}
.sb .l{font-size:.8rem;color:#64748b;margin-top:4px}
.wl{list-style:none}
.wi{display:flex;justify-content:space-between;align-items:center;padding:12px;border-bottom:1px solid #334155}
.wi:last-child{border-bottom:none}
.wn{font-weight:600;color:#f1f5f9}
.wm{font-size:.85rem;color:#94a3b8}
.ws{color:#10b981;font-weight:700}
.bar{height:8px;background:#334155;border-radius:4px;overflow:hidden;margin-top:6px}
.bf{height:100%;background:linear-gradient(90deg,#059669,#10b981);border-radius:4px}
.fg{margin-bottom:12px}
.fg label{display:block;margin-bottom:4px;color:#94a3b8;font-size:.85rem}
.fg input{width:100%;padding:10px;border:1px solid #334155;border-radius:8px;background:#0f172a;color:#e2e8f0;font-size:.95rem}
.fg input:focus{outline:none;border-color:#10b981}
.btn{padding:10px 20px;border:none;border-radius:8px;cursor:pointer;font-weight:600;font-size:.9rem;transition:.2s}
.bp{background:#059669;color:#fff}.bp:hover{background:#047857}
.bd{background:#dc2626;color:#fff}.bd:hover{background:#b91c1c}
.bs{padding:6px 12px;font-size:.8rem}
.tc{display:none}.tc.active{display:block}
.toast{position:fixed;bottom:20px;right:20px;background:#059669;color:#fff;padding:12px 20px;border-radius:8px;font-weight:600;z-index:200}
.sb2{width:100%;padding:10px;border:1px solid #334155;border-radius:8px;background:#0f172a;color:#e2e8f0;margin-bottom:12px}
</style>
</head>
<body>
<div class="hdr"><h1>CREWTRACK</h1><p>Smart Attendance & Payroll Tracker</p></div>
<div class="nav">
<button class="active" onclick="st('dash')">Dashboard</button>
<button onclick="st('wkr')">Workers</button>
<button onclick="st('att')">Attendance</button>
<button onclick="st('sal')">Salary</button>
<button onclick="st('add')">+ Add</button>
<button onclick="st('time')">Time</button>
</div>
<div class="ct">
<div id="t-dash" class="tc active">
<div class="sg">
<div class="sb"><div class="n" id="tc">0</div><div class="l">Today</div></div>
<div class="sb"><div class="n" id="te">0</div><div class="l">Employees</div></div>
<div class="sb"><div class="n" id="lt">--:--</div><div class="l">Last Scan</div></div>
</div>
<div class="card"><h2>Recent Scans</h2><ul class="wl" id="tl"></ul></div>
</div>
<div id="t-wkr" class="tc">
<div class="card"><h2>Employees</h2>
<input type="text" class="sb2" placeholder="Search..." oninput="fw(this.value)">
<ul class="wl" id="wl"></ul></div>
</div>
<div id="t-att" class="tc">
<div class="card"><h2>Attendance History</h2><ul class="wl" id="al"></ul></div>
</div>
<div id="t-sal" class="tc">
<div class="card"><h2>Monthly Salary</h2><ul class="wl" id="sl"></ul></div>
<div style="text-align:center;margin-top:12px"><button class="btn bp" onclick="dlCSV()">Download CSV</button></div>
</div>
<div id="t-add" class="tc">
<div class="card"><h2>Add Employee</h2>
<div class="fg"><label>RFID UID</label><input type="text" id="nu" placeholder="UID"></div>
<div class="fg"><label>Name</label><input type="text" id="nn" placeholder="Name"></div>
<div class="fg"><label>Daily Wage</label><input type="number" id="nw" value="50" step="0.01"></div>
<div class="fg"><label>Phone</label><input type="text" id="np" placeholder="Optional"></div>
<button class="btn bp" onclick="ae()">Add Employee</button></div>
<div class="card" id="ec" style="display:none"><h2>Edit Employee</h2>
<input type="hidden" id="eu">
<div class="fg"><label>Name</label><input type="text" id="en"></div>
<div class="fg"><label>Wage</label><input type="number" id="ew" step="0.01"></div>
<div class="fg"><label>Phone</label><input type="text" id="ep"></div>
<button class="btn bp" onclick="ue()">Save</button>
<button class="btn bd" onclick="re()">Delete</button></div>
</div>
</div>
<div id="t-time" class="tc">
<div class="card"><h2>Set Date & Time</h2>
<div style="margin-bottom:8px;color:#94a3b8;font-size:.85rem">No internet — set manually if timestamps are wrong</div>
<div class="fg"><label>Day</label><input type="number" id="td" min="1" max="31"></div>
<div class="fg"><label>Month</label><input type="number" id="tm" min="1" max="12"></div>
<div class="fg"><label>Year</label><input type="number" id="ty" min="2024" max="2099"></div>
<div class="fg"><label>Hour (0-23)</label><input type="number" id="th" min="0" max="23"></div>
<div class="fg"><label>Minute</label><input type="number" id="tmi" min="0" max="59"></div>
<button class="btn bp" onclick="settime()">Set Time</button>
</div></div>
<div id="toast" class="toast" style="display:none"></div>
<script>
let AW=[];
function st(t){document.querySelectorAll('.tc').forEach(e=>e.classList.remove('active'));document.querySelectorAll('.nav button').forEach(b=>b.classList.remove('active'));document.getElementById('t-'+t).classList.add('active');document.querySelectorAll('.nav button').forEach(b=>{if(b.textContent.toLowerCase().includes(t)||(t==='add'&&b.textContent.includes('+')))b.classList.add('active')});if(t==='dash')ld();if(t==='wkr')lw();if(t==='att')la();if(t==='sal')ls();if(t==='time')lt();}
function toast(m){var t=document.getElementById('toast');t.textContent=m;t.style.display='block';setTimeout(()=>t.style.display='none',3000);}
function api(u,o){return fetch(u,o).then(r=>r.json());}
function ld(){
api('/api/dashboard').then(d=>{document.getElementById('tc').textContent=d.todayCount||0;document.getElementById('te').textContent=d.totalEmployees||0;document.getElementById('lt').textContent=d.lastScanTime||'--:--';});
api('/api/today').then(d=>{var l=document.getElementById('tl');l.innerHTML='';(d.records||[]).reverse().forEach(r=>{l.innerHTML+='<li class="wi"><div><span class="wn">'+r.name+'</span><div class="wm">'+r.uid+'</div></div><span class="wm">'+r.time+'</span></li>';});});
}
function lw(){api('/api/employees').then(d=>{AW=d;rw(d);});}
function rw(l){var e=document.getElementById('wl');e.innerHTML='';l.forEach(w=>{e.innerHTML+='<li class="wi"><div><span class="wn">'+w.name+'</span><div class="wm">'+w.uid+' | '+w.wage+'E/day | '+w.daysWorked+' days</div></div><button class="btn bs bp" onclick="eu(\''+w.uid+'\',\''+w.name+'\','+w.wage+',\''+w.phone+'\')">Edit</button></li>';});}
function fw(q){q=q.toLowerCase();rw(AW.filter(w=>w.name.toLowerCase().includes(q)||w.uid.toLowerCase().includes(q)));}
function eu(uid,n,w,p){document.getElementById('ec').style.display='block';document.getElementById('eu').value=uid;document.getElementById('en').value=n;document.getElementById('ew').value=w;document.getElementById('ep').value=p||'';document.getElementById('t-add').scrollIntoView({behavior:'smooth'});}
function ae(){var u=document.getElementById('nu').value.trim(),n=document.getElementById('nn').value.trim(),w=parseFloat(document.getElementById('nw').value)||50,p=document.getElementById('np').value.trim();if(!u||!n){toast('Enter UID and name');return;}api('/api/employees/add',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({uid:u,name:n,wage:w,phone:p})}).then(d=>{if(d.ok){toast('Added');document.getElementById('nu').value='';document.getElementById('nn').value='';document.getElementById('np').value='';}else toast('Error');});}
function ue(){var u=document.getElementById('eu').value,n=document.getElementById('en').value,w=parseFloat(document.getElementById('ew').value),p=document.getElementById('ep').value;api('/api/employees/update',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({uid:u,name:n,wage:w,phone:p})}).then(d=>{if(d.ok){toast('Updated');document.getElementById('ec').style.display='none';lw();}});}
function re(){var u=document.getElementById('eu').value;if(!confirm('Delete?'))return;api('/api/employees/remove',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({uid:u})}).then(d=>{if(d.ok){toast('Deleted');document.getElementById('ec').style.display='none';lw();}});}
function la(){api('/api/attendance').then(d=>{var l=document.getElementById('al');l.innerHTML='';(d||[]).reverse().forEach(r=>{l.innerHTML+='<li class="wi"><div><span class="wn">'+r.name+'</span><div class="wm">'+r.uid+'</div></div><span class="wm">'+r.date+' '+r.time+'</span></li>';});});}
function ls(){var now=new Date();api('/api/monthly',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({month:now.getMonth()+1,year:now.getFullYear()})}).then(d=>{var l=document.getElementById('sl'),tot=0;l.innerHTML='';d.forEach(s=>{tot+=s.salary;var p=s.days>0?Math.min((s.days/22)*100,100):0;l.innerHTML+='<li class="wi" style="flex-direction:column;align-items:stretch"><div style="display:flex;justify-content:space-between"><span class="wn">'+s.name+'</span><span class="ws">'+s.salary.toFixed(2)+'E</span></div><div class="wm">'+s.days+' days x '+s.wage+'E</div><div class="bar"><div class="bf" style="width:'+p+'%"></div></div></li>';});l.innerHTML+='<li class="wi" style="border-top:2px solid #10b981"><span class="wn">TOTAL</span><span class="ws" style="font-size:1.2rem">'+tot.toFixed(2)+'E</span></li>';});}
function dlCSV(){window.open('/api/export/csv','_blank');}
function settime(){var d=parseInt(document.getElementById('td').value),m=parseInt(document.getElementById('tm').value),y=parseInt(document.getElementById('ty').value),h=parseInt(document.getElementById('th').value),mi=parseInt(document.getElementById('tmi').value);if(!d||!m||!y){toast('Fill date fields');return;}api('/api/settime',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({day:d,month:m,year:y,hour:h||0,minute:mi||0})}).then(r=>{if(r.ok)toast('Time: '+r.date+' '+r.time);else toast('Error: '+r.error);});}
function lt(){api('/api/status').then(d=>{if(d.timeSet){var p=d.date.split('_');document.getElementById('td').value=parseInt(p[2]);document.getElementById('tm').value=parseInt(p[1]);document.getElementById('ty').value=parseInt(p[0]);var t=d.time.split(':');document.getElementById('th').value=parseInt(t[0]);document.getElementById('tmi').value=parseInt(t[1]);}else{var n=new Date();document.getElementById('td').value=n.getDate();document.getElementById('tm').value=n.getMonth()+1;document.getElementById('ty').value=n.getFullYear();document.getElementById('th').value=n.getHours();document.getElementById('tmi').value=n.getMinutes();}});}
ld();
</script>
</body>
</html>
)rawliteral";

void setupWebServer() {
    WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
    server = new WebServer(80);

    server->on("/", HTTP_GET, handleRoot);
    server->on("/api/dashboard", HTTP_GET, handleAPIDashboard);
    server->on("/api/employees", HTTP_GET, handleAPIEmployees);
    server->on("/api/employees/add", HTTP_POST, handleAPIAddEmployee);
    server->on("/api/employees/remove", HTTP_POST, handleAPIRemoveEmployee);
    server->on("/api/employees/update", HTTP_POST, handleAPIUpdateEmployee);
    server->on("/api/today", HTTP_GET, handleAPIToday);
    server->on("/api/monthly", HTTP_POST, handleAPIMonthly);
    server->on("/api/attendance", HTTP_GET, handleAPIAttendance);
    server->on("/api/export/csv", HTTP_GET, handleAPIExportCSV);
    server->on("/api/settime", HTTP_POST, handleAPISetTime);
    server->on("/api/status", HTTP_GET, handleAPIStatus);

    server->begin();
}

void handleRoot() {
    server->send_P(200, "text/html", PAGE_HTML);
}

void handleAPIDashboard() {
    JsonDocument doc;
    doc["todayCount"] = countToday(currentDate);
    doc["lastScanName"] = lastScanName;
    doc["lastScanTime"] = lastScanTimeStr;
    doc["totalEmployees"] = employeeCount;
    doc["date"] = currentDate;
    String out;
    serializeJson(doc, out);
    server->send(200, "application/json", out);
}

void handleAPIEmployees() {
    struct tm t;
    bool gotTime = getLocalTime(&t);
    int curMonth = gotTime ? (t.tm_mon + 1) : 7;
    int curYear = gotTime ? (t.tm_year + 1900) : 2026;

    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();
    for (int i = 0; i < employeeCount; i++) {
        if (!employees[i].active) continue;
        JsonObject obj = arr.createNestedObject();
        obj["uid"] = employees[i].uid;
        obj["name"] = employees[i].name;
        obj["wage"] = employees[i].dailyWage;
        obj["phone"] = employees[i].phone;
        obj["daysWorked"] = getDaysWorked(employees[i].uid, curMonth, curYear);
    }
    String out;
    serializeJson(doc, out);
    server->send(200, "application/json", out);
}

void handleAPIAddEmployee() {
    String body = server->arg("plain");
    JsonDocument doc;
    if (deserializeJson(doc, body)) {
        server->send(400, "application/json", "{\"error\":\"Bad JSON\"}");
        return;
    }
    String uid = doc["uid"] | "";
    String name = doc["name"] | "";
    float wage = doc["wage"] | 50.0f;
    String phone = doc["phone"] | "";
    if (uid.length() == 0 || name.length() == 0) {
        server->send(400, "application/json", "{\"error\":\"Missing fields\"}");
        return;
    }
    if (uid.indexOf(',') >= 0 || uid.indexOf('\n') >= 0 || uid.indexOf('"') >= 0) {
        server->send(400, "application/json", "{\"error\":\"Invalid UID\"}");
        return;
    }
    if (addEmployee(uid, name, wage, phone))
        server->send(200, "application/json", "{\"ok\":true}");
    else
        server->send(500, "application/json", "{\"error\":\"Failed\"}");
}

void handleAPIRemoveEmployee() {
    String body = server->arg("plain");
    JsonDocument doc;
    deserializeJson(doc, body);
    String uid = doc["uid"] | "";
    if (removeEmployee(uid))
        server->send(200, "application/json", "{\"ok\":true}");
    else
        server->send(500, "application/json", "{\"error\":\"Failed\"}");
}

void handleAPIUpdateEmployee() {
    String body = server->arg("plain");
    JsonDocument doc;
    deserializeJson(doc, body);
    String uid = doc["uid"] | "";
    String name = doc["name"] | "";
    float wage = doc["wage"] | 50.0f;
    String phone = doc["phone"] | "";
    if (updateEmployee(uid, name, wage, phone))
        server->send(200, "application/json", "{\"ok\":true}");
    else
        server->send(500, "application/json", "{\"error\":\"Failed\"}");
}

void handleAPIToday() {
    JsonDocument doc;
    doc["date"] = currentDate;
    doc["count"] = countToday(currentDate);
    JsonArray arr = doc.createNestedArray("records");
    String data = readAll(attPath(currentDate));
    int idx = 0;
    while (idx < (int)data.length()) {
        int endIdx = data.indexOf('\n', idx);
        if (endIdx == -1) endIdx = data.length();
        String line = data.substring(idx, endIdx);
        idx = endIdx + 1;
        if (line.length() == 0) continue;
        int c = line.indexOf(',');
        if (c > 0) {
            String uid = line.substring(0, c);
            String time = line.substring(c + 1);
            Employee *emp = findByUID(uid);
            JsonObject obj = arr.createNestedObject();
            obj["uid"] = uid;
            obj["name"] = emp ? emp->name : "Unknown";
            obj["time"] = time;
        }
    }
    String out;
    serializeJson(doc, out);
    server->send(200, "application/json", out);
}

void handleAPIMonthly() {
    String body = server->arg("plain");
    JsonDocument doc;
    deserializeJson(doc, body);
    int month = doc["month"] | 7;
    int year = doc["year"] | 2026;

    JsonDocument result;
    JsonArray arr = result.to<JsonArray>();
    for (int i = 0; i < employeeCount; i++) {
        if (!employees[i].active) continue;
        int days = getDaysWorked(employees[i].uid, month, year);
        JsonObject obj = arr.createNestedObject();
        obj["uid"] = employees[i].uid;
        obj["name"] = employees[i].name;
        obj["days"] = days;
        obj["wage"] = employees[i].dailyWage;
        obj["salary"] = days * employees[i].dailyWage;
    }
    String out;
    serializeJson(result, out);
    server->send(200, "application/json", out);
}

void handleAPIAttendance() {
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();

    if (sdReady) {
        File root = SD.open(ATTENDANCE_DIR);
        if (root) {
            while (File entry = root.openNextFile()) {
                if (entry.isDirectory()) continue;
                String fname = entry.name();
                if (!fname.endsWith(".csv")) continue;
                String date = fname.substring(4, fname.length() - 4);
                String data = "";
                while (entry.available()) data += (char)entry.read();
                entry.close();
                int idx = 0;
                while (idx < (int)data.length()) {
                    int endIdx = data.indexOf('\n', idx);
                    if (endIdx == -1) endIdx = data.length();
                    String line = data.substring(idx, endIdx);
                    idx = endIdx + 1;
                    if (line.length() == 0) continue;
                    int c = line.indexOf(',');
                    if (c > 0) {
                        String uid = line.substring(0, c);
                        String time = line.substring(c + 1);
                        Employee *emp = findByUID(uid);
                        JsonObject obj = arr.createNestedObject();
                        obj["uid"] = uid;
                        obj["name"] = emp ? emp->name : "Unknown";
                        obj["date"] = date;
                        obj["time"] = time;
                    }
                }
            }
            root.close();
        }
    }
    String out;
    serializeJson(doc, out);
    server->send(200, "application/json", out);
}

void handleAPIExportCSV() {
    String csv = "UID,Name,Date,Time,Wage\n";
    if (sdReady) {
        File root = SD.open(ATTENDANCE_DIR);
        if (root) {
            while (File entry = root.openNextFile()) {
                if (entry.isDirectory()) continue;
                String fname = entry.name();
                if (!fname.endsWith(".csv")) continue;
                String date = fname.substring(4, fname.length() - 4);
                String data = "";
                while (entry.available()) data += (char)entry.read();
                entry.close();
                int idx = 0;
                while (idx < (int)data.length()) {
                    int endIdx = data.indexOf('\n', idx);
                    if (endIdx == -1) endIdx = data.length();
                    String line = data.substring(idx, endIdx);
                    idx = endIdx + 1;
                    if (line.length() == 0) continue;
                    int c = line.indexOf(',');
                    if (c > 0) {
                        String uid = line.substring(0, c);
                        String time = line.substring(c + 1);
                        Employee *emp = findByUID(uid);
                        csv += uid + "," + (emp ? emp->name : "Unknown") + "," + date + "," + time + "," + String(emp ? emp->dailyWage : 0) + "\n";
                    }
                }
            }
            root.close();
        }
    }
    server->sendHeader("Content-Disposition", "attachment; filename=attendance.csv");
    server->send(200, "text/csv", csv);
}

void handleAPISetTime() {
    String body = server->arg("plain");
    JsonDocument doc;
    if (deserializeJson(doc, body)) {
        server->send(400, "application/json", "{\"error\":\"Bad JSON\"}");
        return;
    }
    int day = doc["day"] | 1;
    int month = doc["month"] | 1;
    int year = doc["year"] | 2026;
    int hour = doc["hour"] | 0;
    int minute = doc["minute"] | 0;

    if (month < 1 || month > 12 || day < 1 || day > 31 || year < 2024 || year > 2099) {
        server->send(400, "application/json", "{\"error\":\"Invalid date\"}");
        return;
    }

    char dateBuf[16];
    sprintf(dateBuf, "%d_%02d_%02d", year, month, day);
    currentDate = String(dateBuf);

    char timeBuf[8];
    sprintf(timeBuf, "%02d:%02d", hour, minute);
    currentTime = String(timeBuf);

    timeSet = true;
    showReady();

    JsonDocument resp;
    resp["ok"] = true;
    resp["date"] = currentDate;
    resp["time"] = currentTime;
    String out;
    serializeJson(resp, out);
    server->send(200, "application/json", out);

    Serial.print("Time set: ");
    Serial.print(currentDate);
    Serial.print(" ");
    Serial.println(currentTime);
}

void handleAPIStatus() {
    JsonDocument doc;
    doc["date"] = currentDate;
    doc["time"] = currentTime;
    doc["timeSet"] = timeSet;
    doc["sdReady"] = sdReady;
    doc["employeeCount"] = employeeCount;
    doc["ip"] = WiFi.softAPIP().toString();
    doc["uptime"] = millis() / 1000;
    String out;
    serializeJson(doc, out);
    server->send(200, "application/json", out);
}

// ======================== SETUP ========================
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("CrewTrack starting...");

    pinMode(BUZZER, OUTPUT);
    digitalWrite(BUZZER, LOW);
    Serial.println("1. Buzzer OK");

    tft.init();
    tft.setRotation(1);
    tft.setTextDatum(MC_DATUM);
    showBoot("Initializing...");
    Serial.println("2. TFT OK");

    rfid.PCD_Init();
    delay(100);
    Serial.println("3. RFID OK");

    showBoot("Loading SD...");
    if (!SD.begin(SD_CS)) {
        Serial.println("4. SD not found - running without it");
    } else {
        SD.mkdir(ATTENDANCE_DIR);
        sdReady = true;
        Serial.println("4. SD OK");
    }

    showBoot("Loading employees...");
    loadEmployees();
    Serial.print("5. Employees: ");
    Serial.println(employeeCount);

    showBoot("Starting WiFi...");
    setupWebServer();
    Serial.println("6. WiFi OK");

    configTime(GMT_OFFSET_SEC, DST_OFFSET_SEC, NTP_SERVER);
    delay(2000);

    currentDate = getDate();
    currentTime = getTimeStr();

    Serial.print("WiFi IP: ");
    Serial.println(WiFi.softAPIP());

    showReady();
    Serial.println("7. CrewTrack ready!");
    Serial.print("   Date: ");
    Serial.print(currentDate);
    Serial.print(" Time: ");
    Serial.println(currentTime);
}

// ======================== LOOP ========================
void loop() {
    server->handleClient();

    static unsigned long lastClock = 0;
    if (millis() - lastClock > 30000) {
        lastClock = millis();
        currentDate = getDate();
        currentTime = getTimeStr();
    }

    if (currentScreen != SCREEN_READY && (millis() - screenChangeTime > SCREEN_TIMEOUT_MS)) {
        showReady();
    }

    String uid = rfidScan();
    if (uid.length() == 0) return;

    unsigned long now = millis();
    if (now - lastScanTime < DEBOUNCE_MS) return;
    lastScanTime = now;

    currentTime = getTimeStr();
    Serial.print("Scanned: ");
    Serial.println(uid);

    Employee *emp = findByUID(uid);

    if (emp) {
        if (isDuplicateScan(uid, currentDate)) {
            showDuplicate(emp->name);
            Serial.println("Duplicate");
        } else {
            logAttendance(uid, currentDate, currentTime);
            lastScanName = emp->name;
            lastScanTimeStr = currentTime;
            showSuccess(emp->name);
            Serial.print("Logged: ");
            Serial.println(emp->name);
        }
    } else {
        showUnknown(uid);
        Serial.println("Unknown card");
    }
}
