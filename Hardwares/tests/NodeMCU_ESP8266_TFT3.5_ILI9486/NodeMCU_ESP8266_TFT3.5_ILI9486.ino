/**********************************************************************************
 * پروژه تست و راه‌اندازی نمایشگر ۳.۵ اینچ TFT تمام‌رنگ SPI با درایور ILI9486L و NodeMCU ESP8266
 * 
 * سخت‌افزارهای مورد استفاده (از انبار قطعات):
 * 1. ماژول نمایشگر TFT تمام رنگ 3.5 اینچ دارای ارتباط SPI و چیپ درایور ILI9486L (کد کالا: LCD-01-180)
 *    - درایور کنترلر: ILI9486L / ILI9488
 *    - رزولوشن: 480x320 پیکسل (HVGA تمام رنگ ۱۶ بیتی)
 *    - رابط: ارتباط سریال پرسرعت 4-Wire SPI (کانکتور ۸ پین)
 * 2. برد توسعه NodeMcu دارای هسته وای‌فای ESP8266 و مبدل CH340 (کد کالا: COM-03-009)
 * 
 * کتابخانه‌های مورد نیاز:
 * - Adafruit GFX Library (نصب از طریق Arduino Library Manager)
 * - درایور اختصاصی ILI9486_SPI.h موجود در همین پوشه پروژه
 * 
 * جدول سیم‌بندی پین به پین (Wiring Pinout - 8 Pin Header):
 * ---------------------------------------------------------------------------------
 * پایه ماژول نمایشگر (8-Pin) | پایه برد NodeMCU (ESP8266) | شماره GPIO | نوع سیگنال و عملکرد
 * ---------------------------------------------------------------------------------
 * 1. GND                     | GND                     | -          | زمین مشترک مدار
 * 2. VCC                     | 3V3 (یا VIN 5V)         | -          | تغذیه مدار منطقی
 * 3. SCL                     | D5                      | GPIO14     | کلاک سخت‌افزاری HSPI SCK
 * 4. SDA                     | D7                      | GPIO13     | دیتای ارسالی HSPI MOSI
 * 5. RES                     | D0 (یا RST)             | GPIO16     | ریست سخت‌افزاری نمایشگر
 * 6. DC                      | D1                      | GPIO5      | انتخاب دیتا یا فرمان (Data/Command)
 * 7. CS                      | D8                      | GPIO15     | فعال‌ساز نمایشگر (Chip Select)
 * 8. BLK                     | 3V3 (یا D2 جهت دیمر)    | -          | نور پس‌زمینه (Backlight)
 * ---------------------------------------------------------------------------------
 **********************************************************************************/

#include <SPI.h>
#include <Adafruit_GFX.h>
#include "ILI9486_SPI.h"

// =================================================================================
// 1. تعریف پایه‌های اتصال سخت‌افزاری به NodeMCU
// =================================================================================
#define TFT_CS   D8   // GPIO15 (دارای پول‌داون داخلی در نودام‌سی‌یو)
#define TFT_DC   D1   // GPIO5
#define TFT_RST  D0   // GPIO16 (در صورت اتصال به پین RST نودام‌سی‌یو مقدار 1- بگذارید)
#define TFT_BLK  D2   // GPIO4 (اختیاری جهت کنترل روشنایی با PWM یا اتصال مستقیم به 3V3)

// راه‌اندازی شیء نمایشگر با درایور ILI9486 SPI
ILI9486_SPI tft = ILI9486_SPI(TFT_CS, TFT_DC, TFT_RST);

// زمان‌بندی تغییر خودکار صفحات دمو
unsigned long lastSwitchTime = 0;
int currentDemoPage = 0;
const int TOTAL_DEMO_PAGES = 5;
const unsigned long PAGE_DURATION_MS = 6000; // توقف ۶ ثانیه روی هر صفحه

// متغیرهای انیمیشن داشبورد
float gaugeAngle = 0;
int animCounter = 0;

// =================================================================================
// صفحه دمو ۱: داشبورد مشخصات سیستم و کارت هویت (System Specs Card)
// =================================================================================
void drawSystemSpecsPage() {
  tft.fillScreen(0x0842); // پس‌زمینه آبی بسیار تیره

  // کادر سربرگ بالا
  tft.fillRect(0, 0, 480, 45, 0x18E3);
  tft.drawFastHLine(0, 45, 480, COLOR_CYAN);

  tft.setTextSize(2);
  tft.setTextColor(COLOR_WHITE);
  tft.setCursor(15, 12);
  tft.println("NodeMCU ESP8266 + 3.5\" TFT SPI");

  tft.setTextSize(1);
  tft.setTextColor(COLOR_CYAN);
  tft.setCursor(350, 16);
  tft.println("ILI9486L / 480x320");

  // کارت اول: مشخصات پردازنده ESP8266
  int cardY = 60;
  tft.fillRoundRect(15, cardY, 215, 175, 8, 0x10A2);
  tft.drawRoundRect(15, cardY, 215, 175, 8, 0x3186);

  tft.setTextSize(2);
  tft.setTextColor(COLOR_YELLOW);
  tft.setCursor(28, cardY + 14);
  tft.println("ESP8266 MCU");

  tft.setTextSize(1);
  tft.setTextColor(COLOR_WHITE);
  tft.setCursor(28, cardY + 45);
  tft.printf("CPU Freq: %d MHz\n", ESP.getCpuFreqMHz());
  tft.setCursor(28, cardY + 68);
  tft.printf("Chip ID:  0x%08X\n", ESP.getChipId());
  tft.setCursor(28, cardY + 91);
  tft.printf("Flash:    %d MB\n", ESP.getFlashChipRealSize() / (1024 * 1024));
  tft.setCursor(28, cardY + 114);
  tft.printf("Free RAM: %d KB\n", ESP.getFreeHeap() / 1024);
  tft.setCursor(28, cardY + 137);
  tft.printf("Core Ver: %s\n", ESP.getCoreVersion().c_str());

  // کارت دوم: مشخصات ماژول نمایشگر ILI9486
  tft.fillRoundRect(250, cardY, 215, 175, 8, 0x10A2);
  tft.drawRoundRect(250, cardY, 215, 175, 8, 0x3186);

  tft.setTextSize(2);
  tft.setTextColor(COLOR_GREEN);
  tft.setCursor(263, cardY + 14);
  tft.println("3.5\" TFT LCD");

  tft.setTextSize(1);
  tft.setTextColor(COLOR_WHITE);
  tft.setCursor(263, cardY + 45);
  tft.println("Driver:   ILI9486L / ILI9488");
  tft.setCursor(263, cardY + 68);
  tft.println("Pixels:   480 x 320 HVGA");
  tft.setCursor(263, cardY + 91);
  tft.println("Colors:   65K (18-bit SPI)");
  tft.setCursor(263, cardY + 114);
  tft.println("Bus:      HSPI @ 40 MHz");
  tft.setCursor(263, cardY + 137);
  tft.println("Pins:     8-Pin Header");

  // نوار پایینی راهنما
  tft.fillRect(0, 280, 480, 40, 0x10A2);
  tft.drawFastHLine(0, 280, 480, 0x3186);

  tft.setTextSize(1);
  tft.setTextColor(COLOR_ORANGE);
  tft.setCursor(20, 294);
  tft.println("[STATUS] Hardware Initialized Successfully! Auto-cycle running...");
}

// =================================================================================
// صفحه دمو ۲: نوارهای طیف رنگ اصلی (16-bit RGB Color Bars)
// =================================================================================
void drawColorBarsPage() {
  tft.fillScreen(COLOR_BLACK);

  // سربرگ
  tft.setTextSize(2);
  tft.setTextColor(COLOR_WHITE);
  tft.setCursor(15, 15);
  tft.println("16-Bit RGB565 Color Fidelity Test");

  const uint16_t testColors[] = {
    COLOR_RED, COLOR_GREEN, COLOR_BLUE,
    COLOR_YELLOW, COLOR_CYAN, COLOR_MAGENTA,
    COLOR_WHITE, COLOR_ORANGE, COLOR_DARKGREEN, COLOR_NAVY
  };
  const char* colorNames[] = {
    "RED", "GREEN", "BLUE", "YELLOW", "CYAN", "MAGENTA", "WHITE", "ORANGE", "DK_GRN", "NAVY"
  };

  int numBars = 10;
  int barW = 460 / numBars;
  int barH = 120;
  int startX = 10;
  int startY = 50;

  for (int i = 0; i < numBars; i++) {
    tft.fillRect(startX + (i * barW), startY, barW - 2, barH, testColors[i]);
    tft.drawRect(startX + (i * barW), startY, barW - 2, barH, COLOR_WHITE);

    // چاپ نام رنگ به صورت عمودی
    tft.setTextSize(1);
    tft.setTextColor(COLOR_WHITE);
    tft.setCursor(startX + (i * barW) + 4, startY + barH + 10);
    tft.println(colorNames[i]);
  }

  // گرادیانت پله‌ای خاکستری (Grayscale Ramp)
  tft.setTextSize(1);
  tft.setTextColor(COLOR_LIGHTGREY);
  tft.setCursor(12, 205);
  tft.println("32-Step Grayscale DAC Linearity:");

  int rampY = 222;
  int stepW = 460 / 32;
  for (int i = 0; i < 32; i++) {
    uint8_t c = i * 8;
    uint16_t gray = tft.color565(c, c, c);
    tft.fillRect(10 + (i * stepW), rampY, stepW, 35, gray);
  }
  tft.drawRect(9, rampY - 1, (stepW * 32) + 2, 37, COLOR_WHITE);

  tft.setCursor(15, 275);
  tft.setTextColor(COLOR_CYAN);
  tft.println("18-bit SPI Color Protocol: True RGB666 without color distortion.");
}

// =================================================================================
// صفحه دمو ۳: بنچ‌مارک اشکال هندسی با وضوح ۴۸۰×۳۲۰ (Geometric Primitives)
// =================================================================================
void drawGeometryBenchmarkPage() {
  tft.fillScreen(0x0000);

  // سربرگ
  tft.fillRect(0, 0, 480, 32, 0x2124);
  tft.setTextSize(1);
  tft.setTextColor(COLOR_YELLOW);
  tft.setCursor(15, 10);
  tft.println("GEOMETRIC RENDERING BENCHMARK @ 480 x 320 RESOLUTION");

  // ۱. دایره‌های تودرتو
  int cx = 85;
  int cy = 135;
  for (int r = 10; r <= 70; r += 8) {
    tft.drawCircle(cx, cy, r, COLOR_CYAN);
  }
  tft.fillCircle(cx, cy, 6, COLOR_RED);

  // ۲. مستطیل‌های زاویه‌دار گرد (Rounded Rectangles)
  int rx = 240;
  int ry = 135;
  for (int s = 10; s <= 65; s += 9) {
    tft.drawRoundRect(rx - s, ry - (s * 0.7), s * 2, s * 1.4, 6, COLOR_GREENYELLOW);
  }

  // ۳. خطوط شعاعی زاویه‌دار (Star Burst Rays)
  int sx = 395;
  int sy = 135;
  for (int angle = 0; angle < 360; angle += 15) {
    float rad = angle * 0.0174532925;
    int x2 = sx + (int)(cos(rad) * 65.0);
    int y2 = sy + (int)(sin(rad) * 65.0);
    tft.drawLine(sx, sy, x2, y2, COLOR_MAGENTA);
  }

  // مثلث‌های تو در تو در پایین
  tft.drawTriangle(120, 290, 240, 215, 360, 290, COLOR_ORANGE);
  tft.drawTriangle(160, 285, 240, 235, 320, 285, COLOR_YELLOW);
  tft.fillTriangle(210, 280, 240, 255, 270, 280, COLOR_WHITE);

  tft.setTextSize(1);
  tft.setTextColor(COLOR_WHITE);
  tft.setCursor(15, 305);
  tft.println("Hardware SPI HSPI bus transferring up to 40 Mbits/second.");
}

// =================================================================================
// صفحه دمو ۴: شبیه‌ساز داشبورد صنعتی و گیج آنالوگ اینترنت اشیاء (IoT Gauge Dashboard)
// =================================================================================
void drawIoTDashboardPage() {
  tft.fillScreen(0x0821);

  // سربرگ داشبورد
  tft.fillRect(0, 0, 480, 40, 0x18C3);
  tft.drawFastHLine(0, 40, 480, 0x39E7);

  tft.setTextSize(2);
  tft.setTextColor(COLOR_WHITE);
  tft.setCursor(15, 10);
  tft.println("IoT Industrial Telemetry Hub");

  tft.setTextSize(1);
  tft.setTextColor(COLOR_GREEN);
  tft.setCursor(380, 14);
  tft.println("NODE: ONLINE");

  // پنل سمت چپ: گیج آنالوگ عقربه‌ای
  int gx = 130;
  int gy = 160;
  int gr = 85;

  tft.drawCircle(gx, gy, gr, COLOR_WHITE);
  tft.drawCircle(gx, gy, gr - 1, 0x4208);
  tft.drawCircle(gx, gy, gr - 5, COLOR_CYAN);

  // درجات گیج
  for (int a = 150; a <= 390; a += 30) {
    float rad = a * 0.0174533;
    int x1 = gx + (int)(cos(rad) * (gr - 8));
    int y1 = gy + (int)(sin(rad) * (gr - 8));
    int x2 = gx + (int)(cos(rad) * gr);
    int y2 = gy + (int)(sin(rad) * gr);
    tft.drawLine(x1, y1, x2, y2, COLOR_YELLOW);
  }

  // رسم عقربه گیج
  float needleRad = (240.0 + (sin(millis() / 800.0) * 80.0)) * 0.0174533;
  int nx = gx + (int)(cos(needleRad) * (gr - 18));
  int ny = gy + (int)(sin(needleRad) * (gr - 18));
  tft.drawLine(gx, gy, nx, ny, COLOR_RED);
  tft.fillCircle(gx, gy, 5, COLOR_WHITE);

  tft.setTextSize(2);
  tft.setTextColor(COLOR_CYAN);
  tft.setCursor(gx - 35, gy + 45);
  int val = (int)(50.0 + (sin(millis() / 800.0) * 45.0));
  tft.printf("%02d PSI", val);

  // پنل‌های دیجیتال سمت راست
  int rx = 270;
  
  // کارت ۱: پردازنده
  tft.fillRoundRect(rx, 55, 195, 60, 6, 0x10A2);
  tft.drawRoundRect(rx, 55, 195, 60, 6, COLOR_CYAN);
  tft.setTextSize(1);
  tft.setTextColor(COLOR_LIGHTGREY);
  tft.setCursor(rx + 12, 65);
  tft.println("CPU FREQUENCY");
  tft.setTextSize(2);
  tft.setTextColor(COLOR_GREEN);
  tft.setCursor(rx + 12, 85);
  tft.printf("%d MHz (OK)", ESP.getCpuFreqMHz());

  // کارت ۲: حافظه در دسترس
  tft.fillRoundRect(rx, 125, 195, 60, 6, 0x10A2);
  tft.drawRoundRect(rx, 125, 195, 60, 6, COLOR_YELLOW);
  tft.setTextSize(1);
  tft.setTextColor(COLOR_LIGHTGREY);
  tft.setCursor(rx + 12, 135);
  tft.println("HEAP MEMORY");
  tft.setTextSize(2);
  tft.setTextColor(COLOR_YELLOW);
  tft.setCursor(rx + 12, 155);
  tft.printf("%d KB Free", ESP.getFreeHeap() / 1024);

  // کارت ۳: زمان روشن بودن سیستم
  tft.fillRoundRect(rx, 195, 195, 60, 6, 0x10A2);
  tft.drawRoundRect(rx, 195, 195, 60, 6, COLOR_MAGENTA);
  tft.setTextSize(1);
  tft.setTextColor(COLOR_LIGHTGREY);
  tft.setCursor(rx + 12, 205);
  tft.println("SYSTEM UPTIME");
  tft.setTextSize(2);
  tft.setTextColor(COLOR_MAGENTA);
  tft.setCursor(rx + 12, 225);
  tft.printf("%lu sec", millis() / 1000);

  // نوار بارگراف سطح سیگنال
  tft.fillRect(15, 275, 450, 26, 0x10A2);
  tft.drawRect(15, 275, 450, 26, COLOR_WHITE);
  int fillW = (millis() / 50) % 440;
  tft.fillRect(17, 277, fillW, 22, COLOR_GREEN);
}

// =================================================================================
// صفحه دمو ۵: تست فونت و اندازه‌های متنی (Typography & Scaling)
// =================================================================================
void drawTypographyPage() {
  tft.fillScreen(COLOR_BLACK);

  tft.setTextSize(1);
  tft.setTextColor(COLOR_CYAN);
  tft.setCursor(15, 15);
  tft.println("Adafruit_GFX Font Scaling & Print Test:");

  tft.setTextSize(1);
  tft.setTextColor(COLOR_WHITE);
  tft.setCursor(15, 40);
  tft.println("Size 1: 3.5\" TFT Display ILI9486L @ 480x320 pixels");

  tft.setTextSize(2);
  tft.setTextColor(COLOR_GREEN);
  tft.setCursor(15, 65);
  tft.println("Size 2: High Clarity Fonts");

  tft.setTextSize(3);
  tft.setTextColor(COLOR_YELLOW);
  tft.setCursor(15, 100);
  tft.println("Size 3: NodeMCU IoT");

  tft.setTextSize(4);
  tft.setTextColor(COLOR_ORANGE);
  tft.setCursor(15, 145);
  tft.println("Size 4: ESP8266");

  tft.setTextSize(5);
  tft.setTextColor(COLOR_RED);
  tft.setCursor(15, 205);
  tft.println("Size 5: OK!");

  // متن با پس‌زمینه رنگی
  tft.setTextSize(2);
  tft.setTextColor(COLOR_BLACK, COLOR_WHITE);
  tft.setCursor(15, 275);
  tft.println(" INVERTED HIGHLIGHT TEXT TEST ");
}

// =================================================================================
// راه‌اندازی اولیه (Setup)
// =================================================================================
void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println(F("=================================================================="));
  Serial.println(F("  NodeMCU ESP8266 + 3.5\" TFT LCD Test Suite (ILI9486L SPI)       "));
  Serial.println(F("  Resolution: 480 x 320 Pixels | Bus: HSPI 40 MHz                "));
  Serial.println(F("=================================================================="));
  Serial.printf("Chip ID:         0x%08X\n", ESP.getChipId());
  Serial.printf("CPU Frequency:   %d MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("Flash Chip Size: %d Bytes\n", ESP.getFlashChipRealSize());
  Serial.printf("Free Heap:       %d Bytes\n", ESP.getFreeHeap());
  Serial.println(F("Initializing ILI9486 SPI Display Controller..."));

  // راه‌اندازی پین بک‌لایت در صورت استفاده از D2
  #ifdef TFT_BLK
  pinMode(TFT_BLK, OUTPUT);
  digitalWrite(TFT_BLK, HIGH); // روشنایی حداکثر
  #endif

  // راه‌اندازی نمایشگر
  tft.begin(40000000); // فرکانس کلاک SPI برابر ۴۰ مگاهرتز
  tft.setRotation(1);   // حالت لنداسکیپ (480x320)

  Serial.println(F("Display initialized successfully!"));
  Serial.println(F("Type '1' to '5' in Serial Monitor to force jump to any demo page."));

  // اجرای صفحه اول
  drawSystemSpecsPage();
  lastSwitchTime = millis();
}

// =================================================================================
// حلقه اصلی برنامه (Main Loop)
// =================================================================================
void loop() {
  // ۱. بررسی ورودی سریال برای سوئیچ دستی به هر صفحه
  if (Serial.available() > 0) {
    char ch = Serial.read();
    if (ch >= '1' && ch <= '5') {
      currentDemoPage = ch - '1';
      Serial.printf("[MANUAL] Switching to Demo Page %d\n", currentDemoPage + 1);
      renderCurrentPage();
      lastSwitchTime = millis();
    }
  }

  // ۲. تغییر خودکار دوره‌ای صفحات دمو
  if (millis() - lastSwitchTime >= PAGE_DURATION_MS) {
    currentDemoPage = (currentDemoPage + 1) % TOTAL_DEMO_PAGES;
    renderCurrentPage();
    lastSwitchTime = millis();
  }

  delay(20);
}

// =================================================================================
// تابع انتخاب و رندر صفحه جاری
// =================================================================================
void renderCurrentPage() {
  Serial.printf("[DISPLAY] Rendering Page %d of %d...\n", currentDemoPage + 1, TOTAL_DEMO_PAGES);
  switch (currentDemoPage) {
    case 0:
      drawSystemSpecsPage();
      break;
    case 1:
      drawColorBarsPage();
      break;
    case 2:
      drawGeometryBenchmarkPage();
      break;
    case 3:
      drawIoTDashboardPage();
      break;
    case 4:
      drawTypographyPage();
      break;
  }
}
