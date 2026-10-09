/**********************************************************************************
 * پروژه تست و راه‌اندازی ماژول نمایشگر 2.8 اینچ TFT لمسی SPI به همراه NodeMCU ESP8266
 * 
 * سخت‌افزارهای مورد استفاده (از انبار قطعات):
 * 1. ماژول نمایشگر 2.8 اینچ TFT لمسی تمام رنگ SPI ورژن 1.2 (کد کالا: LCD-01-280)
 *    - درایور تصویر: ILI9341 (رزولوشن 320x240 پیکسل، 65K رنگ RGB565)
 *    - کنترلر لمسی: XPT2046 (ارتباط SPI مجزا)
 * 2. برد توسعه NodeMcu دارای هسته وای‌فای ESP8266 و مبدل CH340 (کد کالا: COM-03-009)
 * 
 * کتابخانه‌های مورد نیاز (نصب از طریق Arduino Library Manager):
 * - Adafruit GFX Library (توسط Adafruit)
 * - Adafruit ILI9341 (توسط Adafruit)
 * - XPT2046_Touchscreen (توسط Paul Stoffregen)
 * 
 * اتصالات پین‌ها (Wiring Pinout):
 * ---------------------------------------------------------------------------------
 * پایه نمایشگر (TFT & Touch)  | پایه NodeMCU (ESP8266) | عملکرد پین
 * ---------------------------------------------------------------------------------
 * VCC                        | 3V3                     | تغذیه 3.3 ولت
 * GND                        | GND                     | زمین مدار
 * CS                         | D8 (GPIO15)             | انتخاب تراشه نمایشگر (TFT Chip Select)
 * RESET                      | D0 (GPIO16)             | ریست سخت‌افزاری نمایشگر
 * DC (or RS)                 | D1 (GPIO5)              | انتخاب دستور / داده (Data/Command)
 * SDI (MOSI)                 | D7 (GPIO13)             | گذرگاه سخت‌افزاری SPI MOSI (مشترک)
 * SCK (CLK)                  | D5 (GPIO14)             | کلاک سخت‌افزاری SPI SCK (مشترک)
 * LED (BLK)                  | 3V3                     | نور پس‌زمینه (روشن دائم)
 * SDO (MISO)                 | D6 (GPIO12)             | خروجی تصویر (اختیاری - در صورت عدم نیاز آزاد)
 * T_CLK                      | D5 (GPIO14)             | کلاک تاچ SPI (مشترک با SCK نمایشگر)
 * T_CS                       | D2 (GPIO4)              | انتخاب تراشه تاچ (Touch Chip Select)
 * T_DIN                      | D7 (GPIO13)             | دیتای ورودی تاچ (مشترک با MOSI نمایشگر)
 * T_DO                       | D6 (GPIO12)             | دیتای خروجی تاچ (متصل به MISO)
 * T_IRQ                      | D3 (GPIO0) یا نامتصل    | وقفه تاچ (اختیاری)
 * ---------------------------------------------------------------------------------
 **********************************************************************************/

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>

// =================================================================================
// 1. تعریف پایه‌های اتصال (Pin Definitions)
// =================================================================================
#define TFT_CS    D8    // GPIO15
#define TFT_DC    D1    // GPIO5
#define TFT_RST   D0    // GPIO16 (در صورت اتصال به پایه RST نودام‌سی‌یو، مقدار 1- قرار دهید)

#define TOUCH_CS  D2    // GPIO4
#define TOUCH_IRQ D3    // GPIO0 (اختیاری؛ برای حالت بدون وقفه نامتصل و مقدار 255 بگذارید)

// ساخت اشیاء نمایشگر و تاچ
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen ts(TOUCH_CS); // در صورت استفاده از IRQ: XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);

// =================================================================================
// 2. تنظیمات کالیبراسیون تاچ (Touch Calibration Settings)
// مقادیر خام مبدل آنالوگ به دیجیتال 12 بیتی (200 الی 3800)
// =================================================================================
#define TS_MINX 240
#define TS_MAXX 3820
#define TS_MINY 260
#define TS_MAXY 3800

// تنظیمات چرخش و تطبیق محورها (در صورت لزوم جهت اصلاح لمس تغییر دهید)
#define ROTATION_MODE 1   // 1 = Landscape (افقی 320x240)
#define INVERT_TOUCH_X false
#define INVERT_TOUCH_Y false
#define SWAP_TOUCH_XY  false

// =================================================================================
// 3. پالت رنگ‌ها و تعریف اجزای رابط کاربری (UI)
// =================================================================================
#define COLOR_BG          0x0821  // خاکستری بسیار تیره متمایل به سورمه‌ای
#define COLOR_HEADER      0x18E3  // نوار سربرگ تیره
#define COLOR_PANEL       0x2124  // پنل ابزارها
#define COLOR_CANVAS      0x0000  // بوم نقاشی مشکی مطلق
#define COLOR_BORDER      0x4A69  // رنگ کادرهای بیرونی
#define COLOR_TEXT        0xFFFF  // سفید
#define COLOR_ACCENT      0x07E0  // سبز نئونی
#define COLOR_TOUCH_DOT   0xF800  // قرمز نشانگر لمس

// رنگ‌های قابل انتخاب برای براش قلم
const uint16_t PALETTE_COLORS[] = {
  ILI9341_WHITE,
  ILI9341_RED,
  ILI9341_GREEN,
  ILI9341_BLUE,
  ILI9341_YELLOW,
  ILI9341_CYAN,
  ILI9341_MAGENTA,
  ILI9341_ORANGE
};
const int NUM_COLORS = 8;
uint16_t currentColor = ILI9341_YELLOW;
int brushSize = 3;

// ابعاد ناحیه بوم نقاشی (Canvas Area)
const int CANVAS_X = 65;
const int CANVAS_Y = 38;
const int CANVAS_W = 250;
const int CANVAS_H = 196;

// متغیرهای ذخیره وضعیت آخرین لمس
int lastTouchX = -1;
int lastTouchY = -1;
unsigned long lastTouchTime = 0;
bool isTouching = false;

// =================================================================================
// توابع تبدیل و نگاشت مختصات لمسی (Coordinate Mapping)
// =================================================================================
bool getTouchCoordinates(int &screenX, int &screenY, int &rawX, int &rawY, int &rawZ) {
  if (!ts.touched()) {
    return false;
  }

  TS_Point p = ts.getPoint();
  rawX = p.x;
  rawY = p.y;
  rawZ = p.z;

  // فیلتر کردن نویزهای با فشار بسیار کم
  if (rawZ < 300) {
    return false;
  }

  int tx = rawX;
  int ty = rawY;

  if (SWAP_TOUCH_XY) {
    int temp = tx;
    tx = ty;
    ty = temp;
  }

  // نگاشت مختصات خام به ابعاد صفحه بر اساس چرخش لنداسکیپ (Landscape 320x240)
  // در حالت Rotation 1 (ILI9341):
  // معمولاً محور X تاچ متناظر با محور X صفحه و محور Y متناظر با Y صفحه است
  int mappedX = map(tx, TS_MINX, TS_MAXX, 0, tft.width());
  int mappedY = map(ty, TS_MINY, TS_MAXY, 0, tft.height());

  if (INVERT_TOUCH_X) {
    mappedX = tft.width() - mappedX;
  }
  if (INVERT_TOUCH_Y) {
    mappedY = tft.height() - mappedY;
  }

  screenX = constrain(mappedX, 0, tft.width() - 1);
  screenY = constrain(mappedY, 0, tft.height() - 1);

  return true;
}

// =================================================================================
// ترسیم بوم نقاشی (Canvas Drawing Area)
// =================================================================================
void drawCanvasArea() {
  tft.fillRect(CANVAS_X, CANVAS_Y, CANVAS_W, CANVAS_H, COLOR_CANVAS);
  tft.drawRect(CANVAS_X - 1, CANVAS_Y - 1, CANVAS_W + 2, CANVAS_H + 2, COLOR_BORDER);
  
  // متن راهنمای درون بوم
  tft.setTextSize(1);
  tft.setTextColor(0x52AA);
  tft.setCursor(CANVAS_X + 25, CANVAS_Y + CANVAS_H / 2 - 5);
  tft.println("TOUCH / DRAW ON THIS CANVAS");
}

// =================================================================================
// ترسیم سربرگ و نوار بالایی (Header Bar)
// =================================================================================
void drawHeader(const char* title, uint16_t titleColor = ILI9341_WHITE) {
  tft.fillRect(0, 0, 320, 32, COLOR_HEADER);
  tft.drawFastHLine(0, 32, 320, COLOR_BORDER);

  tft.setTextSize(1);
  tft.setTextColor(COLOR_ACCENT);
  tft.setCursor(6, 6);
  tft.print("NodeMCU ESP8266 + 2.8\" TFT SPI");

  tft.setTextColor(titleColor);
  tft.setCursor(6, 18);
  tft.print(title);

  // نشانگر وضعیت اتصال لمسی در گوشه بالا راست
  updateTouchStatusIndicator(false, 0, 0);
}

// =================================================================================
// به‌روزرسانی زنده وضعیت و مختصات لمس در سربرگ
// =================================================================================
void updateTouchStatusIndicator(bool active, int sx, int sy) {
  tft.fillRect(210, 4, 105, 24, COLOR_HEADER);
  tft.drawRect(210, 4, 105, 24, active ? COLOR_ACCENT : COLOR_BORDER);

  tft.setTextSize(1);
  if (active) {
    tft.setTextColor(COLOR_ACCENT);
    tft.setCursor(215, 7);
    tft.print("TOUCH: ON");
    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(215, 17);
    tft.printf("X:%03d Y:%03d", sx, sy);
  } else {
    tft.setTextColor(0x8410);
    tft.setCursor(215, 7);
    tft.print("TOUCH: IDLE");
    tft.setCursor(215, 17);
    tft.print("Tap Screen...");
  }
}

// =================================================================================
// ترسیم نوار ابزار سمت چپ (Left Sidebar Tool Panel)
// =================================================================================
void drawToolPanel() {
  tft.fillRect(0, 33, 62, 207, COLOR_PANEL);
  tft.drawFastVLine(62, 33, 207, COLOR_BORDER);

  // عنوان بخش پالت
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(8, 38);
  tft.print("COLORS");

  // رسم دکمه‌های انتخاب رنگ
  int startY = 50;
  int btnH = 15;
  int btnW = 46;

  for (int i = 0; i < NUM_COLORS; i++) {
    int by = startY + (i * (btnH + 3));
    tft.fillRect(8, by, btnW, btnH, PALETTE_COLORS[i]);
    
    // اگر رنگ جاری باشد، حاشیه ضخیم سفید بکش
    if (PALETTE_COLORS[i] == currentColor) {
      tft.drawRect(6, by - 2, btnW + 4, btnH + 4, ILI9341_WHITE);
      tft.drawRect(7, by - 1, btnW + 2, btnH + 2, ILI9341_BLACK);
    } else {
      tft.drawRect(8, by, btnW, btnH, ILI9341_BLACK);
    }
  }

  // دکمه پاکسازی بوم (CLEAR)
  tft.fillRect(6, 196, 50, 18, 0xC800); // قرمز تیره
  tft.drawRect(6, 196, 50, 18, ILI9341_WHITE);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(14, 201);
  tft.print("CLEAR");

  // دکمه تست گرافیک (TEST)
  tft.fillRect(6, 218, 50, 18, 0x03E0); // سبز متمایل به تیره
  tft.drawRect(6, 218, 50, 18, ILI9341_WHITE);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(17, 223);
  tft.print("TEST");
}

// =================================================================================
// آزمون نمایشی بنچ‌مارک گرافیک (Graphics Benchmark Test)
// =================================================================================
void runGraphicsDemo() {
  Serial.println(F("[DEMO] Starting Graphics Demo Suite..."));
  
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(40, 30);
  tft.println("GRAPHICS BENCHMARK");
  
  tft.setTextSize(1);
  tft.setTextColor(COLOR_ACCENT);
  tft.setCursor(40, 55);
  tft.println("Display Driver: ILI9341 (320x240 SPI)");
  tft.setTextColor(ILI9341_CYAN);
  tft.setCursor(40, 70);
  tft.println("Controller: NodeMCU ESP8266 @ 80/160MHz");

  // 1. میله‌های طیف رنگ اصلی (Color Bars)
  const uint16_t bars[] = {
    ILI9341_RED, ILI9341_GREEN, ILI9341_BLUE, 
    ILI9341_YELLOW, ILI9341_CYAN, ILI9341_MAGENTA, ILI9341_WHITE
  };
  int barW = 300 / 7;
  for (int i = 0; i < 7; i++) {
    tft.fillRect(10 + (i * barW), 90, barW, 25, bars[i]);
  }
  tft.drawRect(9, 89, (barW * 7) + 2, 27, ILI9341_WHITE);

  // 2. ترسیم دایره‌ها و اشکال هم‌مرکز
  int cx = 70;
  int cy = 175;
  for (int r = 10; r <= 45; r += 7) {
    tft.drawCircle(cx, cy, r, ILI9341_YELLOW);
  }
  tft.fillCircle(cx, cy, 6, ILI9341_RED);

  // 3. ترسیم مستطیل‌های تو در تو
  int rx = 160;
  int ry = 175;
  for (int s = 10; s <= 45; s += 7) {
    tft.drawRect(rx - s, ry - s, s * 2, s * 2, ILI9341_CYAN);
  }

  // 4. ترسیم مثلث‌ها و خطوط شعاعی
  int tx = 265;
  int ty = 175;
  for (int i = 0; i < 360; i += 45) {
    float rad = i * 0.0174533;
    int x2 = tx + (int)(cos(rad) * 40.0);
    int y2 = ty + (int)(sin(rad) * 40.0);
    tft.drawLine(tx, ty, x2, y2, ILI9341_GREEN);
  }

  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(55, 228);
  tft.println("TOUCH SCREEN TO RETURN TO CANVAS");

  // انتظار برای لمس صفحه توسط کاربر جهت بازگشت
  unsigned long startWait = millis();
  while (millis() - startWait < 8000) {
    if (ts.touched()) {
      TS_Point p = ts.getPoint();
      if (p.z > 300) {
        break;
      }
    }
    delay(50);
  }

  // بازسازی صفحه کاربری اصلی
  initUserInterface();
}

// =================================================================================
// راه‌اندازی و بازسازی رابط کاربری کامل (UI Initialization)
// =================================================================================
void initUserInterface() {
  tft.fillScreen(COLOR_BG);
  drawHeader("Interactive Paint & Touch Test");
  drawToolPanel();
  drawCanvasArea();
  isTouching = false;
  lastTouchX = -1;
  lastTouchY = -1;
}

// =================================================================================
// مدیریت کلیک روی دکمه‌های نوار ابزار سمت چپ
// =================================================================================
void handleToolPanelTouch(int sx, int sy) {
  // دکمه CLEAR
  if (sx >= 6 && sx <= 56 && sy >= 196 && sy <= 214) {
    Serial.println(F("[ACTION] Clear Canvas pressed."));
    drawCanvasArea();
    delay(150);
    return;
  }

  // دکمه TEST
  if (sx >= 6 && sx <= 56 && sy >= 218 && sy <= 236) {
    Serial.println(F("[ACTION] Test Benchmark pressed."));
    runGraphicsDemo();
    return;
  }

  // بررسی انتخاب رنگ از پالت
  int startY = 50;
  int btnH = 15;
  int btnW = 46;

  for (int i = 0; i < NUM_COLORS; i++) {
    int by = startY + (i * (btnH + 3));
    if (sx >= 8 && sx <= (8 + btnW) && sy >= by && sy <= (by + btnH)) {
      currentColor = PALETTE_COLORS[i];
      Serial.print(F("[ACTION] Color selected: 0x"));
      Serial.println(currentColor, HEX);
      drawToolPanel(); // بازطراحی حاشیه رنگ انتخابی
      delay(100);
      return;
    }
  }
}

// =================================================================================
// تنظیمات اولیه (Setup)
// =================================================================================
void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println(F("========================================================="));
  Serial.println(F("  NodeMCU ESP8266 + 2.8\" SPI TFT Touch Test Program     "));
  Serial.println(F("  Display: ILI9341 (320x240) | Touch: XPT2046 SPI       "));
  Serial.println(F("========================================================="));
  Serial.printf("ESP8266 Chip ID: %08X\n", ESP.getChipId());
  Serial.printf("CPU Frequency:   %d MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("Free Heap:       %d bytes\n", ESP.getFreeHeap());
  Serial.println(F("Initializing SPI Display & Touch Controller..."));

  // راه‌اندازی نمایشگر TFT
  tft.begin();
  tft.setRotation(ROTATION_MODE);
  tft.fillScreen(ILI9341_BLACK);

  // راه‌اندازی کنترلر تاچ XPT2046
  ts.begin();
  ts.setRotation(ROTATION_MODE);

  Serial.println(F("Hardware initialized successfully!"));
  Serial.println(F("Touch screen anywhere or draw on the canvas."));

  // بارگذاری صفحه رابط کاربری
  initUserInterface();
}

// =================================================================================
// حلقه اصلی برنامه (Main Loop)
// =================================================================================
void loop() {
  int screenX = 0;
  int screenY = 0;
  int rawX = 0;
  int rawY = 0;
  int rawZ = 0;

  bool touched = getTouchCoordinates(screenX, screenY, rawX, rawY, rawZ);

  if (touched) {
    lastTouchTime = millis();

    // چاپ مشخصات لمس در مانیتور سریال
    Serial.printf("[TOUCH] Raw: X=%04d, Y=%04d, Z=%04d  -->  Screen: X=%03d, Y=%03d\n",
                  rawX, rawY, rawZ, screenX, screenY);

    // ۱. اگر لمس در محدوده بوم نقاشی باشد (Drawing Canvas)
    if (screenX >= (CANVAS_X + brushSize) && screenX <= (CANVAS_X + CANVAS_W - brushSize) &&
        screenY >= (CANVAS_Y + brushSize) && screenY <= (CANVAS_Y + CANVAS_H - brushSize)) {
      
      if (isTouching && lastTouchX != -1 && lastTouchY != -1) {
        // برای کشیدن خط ممتد و نرم بین دونقطه متوالی
        tft.drawLine(lastTouchX, lastTouchY, screenX, screenY, currentColor);
        tft.fillCircle(screenX, screenY, brushSize, currentColor);
      } else {
        tft.fillCircle(screenX, screenY, brushSize, currentColor);
      }

      lastTouchX = screenX;
      lastTouchY = screenY;
    }
    // ۲. اگر لمس در نوار ابزار سمت چپ باشد
    else if (screenX < CANVAS_X) {
      handleToolPanelTouch(screenX, screenY);
      lastTouchX = -1;
      lastTouchY = -1;
    }

    if (!isTouching) {
      isTouching = true;
      updateTouchStatusIndicator(true, screenX, screenY);
    } else {
      // به‌روزرسانی مختصات زنده
      updateTouchStatusIndicator(true, screenX, screenY);
    }

  } else {
    // در صورت قطع لمس برای بیش از 150 میلی‌ثانیه
    if (isTouching && (millis() - lastTouchTime > 150)) {
      isTouching = false;
      lastTouchX = -1;
      lastTouchY = -1;
      updateTouchStatusIndicator(false, 0, 0);
    }
  }

  delay(10); // تاخیر کوتاه برای پایداری پردازش SPI
}
