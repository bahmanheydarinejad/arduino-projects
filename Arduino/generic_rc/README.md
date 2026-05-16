📡 Universal Modular RC System (Arduino + nRF24L01+)
🎯 معرفی (Overview)
این پروژه یک سیستم کنترل رادیویی (RC) چندمنظوره، ماژولار و مبتنی بر رویداد (Event-driven) است که برای کاربردهای بلادرنگ (Real-time) نهفته طراحی شده است. این سیستم با رعایت اصول Clean Architecture و SOLID ساخته شده و با ارائه تاخیر بسیار پایین ($< 10ms$) و پایداری بالا، برای انواع وسایل نقلیه کنترل از راه دور مناسب است.

موارد استفاده:

🚁 پهپادها (Drones) | 🚙 ماشین‌های RC | 🚤 قایق‌ها | 🤖 رباتیک | 🎮 کنترلرهای عمومی

🧱 معماری سیستم (System Architecture)
سیستم به ۴ لایه منطقی تقسیم شده است:

🔷 ۱. لایه ارتباطی (Communication / RF Layer)
مسئول انتقال پایدار داده‌ها در بستر RF.

سخت‌افزار: nRF24L01+ PA/LNA و بردهای Arduino (Nano, UNO, UNO R4).
ویژگی‌ها: سرعت داده $1 Mbps$، سایز ثابت Payload ($32$ بایت)، Auto-ACK، مکانیزم Retry ($2-5$ بار)، محاسبه کیفیت لینک یا Link Quality ($0-100$).
ارتباط دوطرفه: ارسال فرمان (TX
→
→
RX) و دریافت تله‌متری (RX
→
→
TX).
🧠 ۲. لایه پروتکل (Protocol Layer)
تعریف استاندارد انتقال داده سبک و مستقل از کاربرد با جداسازی دقیق MetaData و Data.

امنیت و یکپارچگی: بررسی با CRC / XOR، شماره توالی (Sequence Number) برای جلوگیری از بسته تکراری و گم‌شده.
Fail-safe (مدیریت خطا): تشخیص Timeout (مثلاً $100 ms$). در صورت قطع ارتباط: تراتل به حداقل ($0$) و سایر کانال‌ها به حالت خنثی (Neutral) می‌روند.
انواع پکت (Type): پشتیبانی از کنترل، تله‌متری و جفت‌سازی (Pairing).
⚙️ ۳. لایه اپلیکیشن (Application Layer)
این لایه کاملاً انتزاعی و Event-driven است و رفتار سیستم را مدل می‌کند.

سیستم مدیریت ورودی (Input Manager):
دکمه‌ها (Buttons): تشخیص Stateهای مختلف (Press, Release, Hold)، Debounce ($30 ms$).
جوی‌استیک (Analog): پشتیبانی از $4$ محور، فیلتر Deadzone، کالیبراسیون مرکز، کاهش نویز.
مقادیر عددی (Range): برای ورودی‌های سریال یا پتانسیومترها.
سیستم خروجی (Output Modes):
PWM: بازه $1000 - 2000 \mu s$ (مناسب سروو و ماشین RC).
PPM: ارسال چند کانال روی یک سیم.
SBUS: پروتکل دیجیتال (مناسب فلایت‌کنترلرها).
📦 ۴. لایه انتزاع سخت‌افزار (Board Abstraction Layer)
پشتیبانی از بردهای مختلف آردوینو بدون نیاز به تغییر کد (استفاده از Factory Pattern).

بردهای پشتیبانی شده: NanoBoard ، UnoBoard ، UnoR4Board.
عملکرد: تخصیص پین‌ها و تنظیمات سخت‌افزاری تنها با تغییر Target کامپایل انجام می‌شود.
📦 ساختار داده (Data Model)
تمامی پکت‌ها برای زمان‌بندی قطعی (Deterministic Timing) سایز ثابت ($32$ بایت) دارند و از عملیات اعشاری (Float) برای حفظ پرفورمنس استفاده نمی‌شود.

cpp
// ساختار متادیتا برای مدیریت پکت
struct MetaData {
uint8_t sync;     // 0xAA
uint8_t seq;      // Sequence number
uint8_t flags;    // Control bits (ARM, FAILSAFE, Mode)
uint8_t type;     // Packet type (Control / Telemetry)
uint8_t len;      // Data length
uint8_t crc;      // Checksum / XOR
} __attribute__((packed));

// ساختار داده‌های کنترلی یکپارچه
struct ControlData {
uint16_t buttons;     // Bitfield for buttons + switches
uint8_t  events[10];  // Button event states
int16_t  axes[4];     // Joystick X/Y pairs
int16_t  ranges[2];   // External numeric inputs
} __attribute__((packed));

// پکت نهایی ارسالی (مجموعاً 32 بایت)
struct Packet {
MetaData meta;
ControlData data;
} __attribute__((packed));
🚀 ویژگی‌های کلیدی سیستم در یک نگاه
معماری ۴ لایه ماژولار با قابلیت مقیاس‌پذیری بالا.
تاخیر بسیار کم ($< 10 ms$) با آپدیت ریت $50 - 200 Hz$.
ارسال مبتنی بر تغییر (Event-driven): جلوگیری از ارسال داده‌های تکراری و اسپم.
سیستم Fail-safe هوشمند در صورت قطع ارتباط.
تخمین کیفیت لینک ارتباطی (نرم‌افزاری بر اساس درصد Packet Loss).
🔮 توسعه‌های آینده (Future Improvements)
اضافه کردن تله‌متری پیشرفته‌تر (ولتاژ باتری، سنسورها).
پیاده‌سازی Frequency Hopping برای پایداری بیشتر.
رمزنگاری (Encryption) و Binding امن بین TX/RX.
فشرده‌سازی کانال‌ها به $11$ بیت (مشابه استاندارد SBUS).
مهاجرت کدهای هسته به میکروکنترلرهای قدرتمندتر مانند STM32 و ESP32.