#include "UIManager.h"
#include <string.h>
#include <stdio.h>
#include "font.h"

/*
 * 界面管理层思路：
 * 1. 用一个枚举表示当前界面 (currentScreen)。
 * 2. 每个界面都有一个专门的"绘制函数" (或称"刷新函数")。
 * 3. UIManager_Update() 调用当前界面的绘制函数。
 * 4. 通过 UIManager_SwitchScreen() 改变 currentScreen。
 */

// ------------------【1】定义当前界面 ------------------

static UIScreen_t currentScreen = SCREEN_MAIN;

// ------------------【2】声明各界面绘制函数 ------------------
// 你可以根据自己需要，写更多界面，如：SCREEN_STATUS, SCREEN_DATA, SCREEN_ENV
static void UI_DrawMain(void);
static void UI_DrawStatus(void);
static void UI_DrawData(void);
static void UI_DrawEnv(void);

// 如果界面较多，可以用函数指针数组来简化
typedef void (*UIDrawFunc_t)(void);
static UIDrawFunc_t s_drawFuncs[SCREEN_MAX] = {
    UI_DrawMain,              // SCREEN_MAIN
    UI_DrawStatus,           // SCREEN_STATUS  
    UI_DrawData,            // SCREEN_DATA
    UI_DrawEnv,             // SCREEN_ENV
    
    UI_scales_1,            // SCREEN_SCALES_1
    UI_scales_2,            // SCREEN_SCALES_2  
    UI_scales_3,            // SCREEN_SCALES_3
    UI_select_fruit,        // SCREEN_SELECT_FRUIT
    UI_hx711_set_calibration, // SCREEN_HX711_CALIBRATION
    
    UI_pay_payment,         // SCREEN_PAYMENT
    UI_pay_success,         // SCREEN_PAY_SUCCESS
    UI_pay_failure,         // SCREEN_PAY_FAILED
    UI_pay_cancel,          // SCREEN_PAY_CANCEL
    
    UI_card_input,          // SCREEN_CARD_INPUT
    UI_card_input_success,  // SCREEN_CARD_INPUT_SUCCESS  
    UI_card_input_failed,   // SCREEN_CARD_INPUT_FAILED
    UI_card_recharge,       // SCREEN_CARD_RECHARGE
    UI_card_balance,        // SCREEN_CARD_BALANCE
    
    UI_net_connect,         // SCREEN_NET_CONNECT
    UI_net_disconnect,      // SCREEN_NET_DISCONNECT
    UI_net_sync,            // SCREEN_NET_SYNC
    
    UI_system_error,        // SCREEN_SYSTEM_ERROR
    UI_system_info          // SCREEN_SYSTEM_INFO};

// ------------------【3】对外接口实现 ------------------
void UIManager_Init(void)
{
    // 初始化时，可做一些界面相关变量的清零
    // 例如：设置默认界面
    currentScreen = SCREEN_MAIN;

    // 也可以直接先画一次
    UIManager_Update();
}

void UIManager_Update(void)
{
    // 调用当前界面的绘制函数
    if (currentScreen < SCREEN_MAX)
    {
        s_drawFuncs[currentScreen]();
    }
}

void UIManager_SwitchScreen(UIScreen_t screen)
{
    if (screen >= SCREEN_MAX)
        return;

    // 修改当前界面
    currentScreen = screen;

    // 立即刷新或等到下次UIManager_Update时再刷新
    UIManager_Update();
}

// ------------------【4】各界面绘制函数实现 ------------------

// ========== 示例：主界面 ==========
static void UI_DrawMain(void)
{
}

// ========== 示例：状态界面 ==========
static void UI_DrawStatus(void)
{
    OLED_NewFrame();
    // 显示标题
    OLED_PrintString(8, 0, "智能水果售货机", &font16x16, OLED_COLOR_NORMAL);

    // 显示提示信息
    OLED_PrintString((128 - (16 * 4)) / 2, 16, "欢迎使用", &font16x16, OLED_COLOR_NORMAL);

    OLED_ShowFrame();
}

// ========== 示例：数据界面 ==========
static void UI_DrawData(void)
{
    OLED_NewFrame();

    // 显示标题
    OLED_PrintString(8, 0, "智能水果售货机", &font16x16, OLED_COLOR_NORMAL);

    // 显示提示信息
    OLED_PrintString((128 - (16 * 5)) / 2, 16, "请放置水果", &font16x16, OLED_COLOR_NORMAL);

    // 显示风扇和LED的状态
    char fanStatusStr[32];
    char ledStatusStr[32];
    sprintf(fanStatusStr, "风扇状态: %s", g_fruitVendingData.fun_status ? "开启" : "关闭");
    sprintf(ledStatusStr, "LED状态: %s", g_fruitVendingData.led_status ? "开启" : "关闭");
    OLED_PrintString(0, 32, fanStatusStr, &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(0, 48, ledStatusStr, &font16x16, OLED_COLOR_NORMAL);

    OLED_ShowFrame();
}

// ========== 示例：环境界面 ==========
static void UI_DrawEnv(void)
{
    OLED_NewFrame();

    // 显示标题
    OLED_PrintString(8, 0, "智能水果售货机", &font16x16, OLED_COLOR_NORMAL);

    // 显示提示信息
    OLED_PrintString((128 - (16 * 5)) / 2, 16, "请放置水果", &font16x16, OLED_COLOR_NORMAL);

    // 显示温湿度信息
    char tempStr[32];
    char humStr[32];
    sprintf(tempStr, "温度: %d°C", g_fruitVendingData.CurrentTemp);
    sprintf(humStr, "湿度: %d%%", g_fruitVendingData.CurrentHum);
    OLED_PrintString(0, 32, tempStr, &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(0, 48, humStr, &font16x16, OLED_COLOR_NORMAL);

    OLED_ShowFrame();
}

static void UI_scales_1(void)
{
    OLED_NewFrame();
    uint32_t price = 0;

    char fruit_name[16];
    sprintf(fruit_name,"秤1:%s",FruitName[g_fruitVendingData.ActionFruit]);

    // 显示标题 以及水果种类
    OLED_PrintString(48, 0, fruit_name, &font16x16, OLED_COLOR_NORMAL);

    // 显示重量
    char weightStr[32];
    sprintf(weightStr, "重量: %dg", g_fruitVendingData.CurrentFruit_1_weight);
    OLED_PrintString(0, 16, weightStr, &font16x16, OLED_COLOR_NORMAL);

    // 显示价格
    char priceStr[32];
    price = (g_fruitVendingData.CurrentFruit_1_weight * FruitPrice[g_fruitVendingData.ActionFruit]) / 1000;
    sprintf(priceStr, "价格: %d.%02d元", price / 100, price % 100);
    OLED_PrintString(0, 32, priceStr, &font16x16, OLED_COLOR_NORMAL);

    // 显示操作提示
    OLED_PrintString(0, 48, "按确认键开始支付", &font16x16, OLED_COLOR_NORMAL);

    // 设置进入支付界面
    g_fruitVendingData.oledshowpage = OLED_SHOW_PAGE_PAYMENT;

    OLED_ShowFrame();
}

static void UI_scales_2(void)
{
    OLED_NewFrame();
    uint32_t price = 0;

    char fruit_name[16];
    sprintf(fruit_name,"秤2:%s",FruitName[g_fruitVendingData.ActionFruit]);

    // 显示标题 以及水果种类
    OLED_PrintString(48, 0, fruit_name, &font16x16, OLED_COLOR_NORMAL);

    // 显示重量
    char weightStr[32];
    sprintf(weightStr, "重量: %dg", g_fruitVendingData.CurrentFruit_2_weight);
    OLED_PrintString(0, 16, weightStr, &font16x16, OLED_COLOR_NORMAL);

    // 显示价格
    char priceStr[32];
    price = (g_fruitVendingData.CurrentFruit_2_weight * FruitPrice[g_fruitVendingData.ActionFruit]) / 1000;
    sprintf(priceStr, "价格: %d.%02d元", price / 100, price % 100);
    OLED_PrintString(0, 32, priceStr, &font16x16, OLED_COLOR_NORMAL);

    // 显示操作提示
    OLED_PrintString(0, 48, "按确认键开始支付", &font16x16, OLED_COLOR_NORMAL);

    // 设置进入支付界面
    g_fruitVendingData.oledshowpage = OLED_SHOW_PAGE_PAYMENT;

    OLED_ShowFrame();
}

static void UI_scales_3(void)
{
    OLED_NewFrame();

    char fruit_name[16];
    sprintf(fruit_name, "秤3:%s", FruitName[g_fruitVendingData.ActionFruit]);

    // 显示标题 以及水果种类
    OLED_PrintString(48, 0, fruit_name, &font16x16, OLED_COLOR_NORMAL);

    // 显示重量
    char weightStr[32];
    sprintf(weightStr, "重量: %dg", g_fruitVendingData.CurrentFruit_3_weight);
    OLED_PrintString(0, 16, weightStr, &font16x16, OLED_COLOR_NORMAL);

    // 显示价格
    char priceStr[32];
    uint32_t price = (g_fruitVendingData.CurrentFruit_3_weight * FruitPrice[g_fruitVendingData.ActionFruit]) / 1000;
    sprintf(priceStr, "价格: %d.%02d元", price / 100, price % 100);
    OLED_PrintString(0, 32, priceStr, &font16x16, OLED_COLOR_NORMAL);

    // 显示操作提示
    OLED_PrintString(0, 48, "按确认键开始支付", &font16x16, OLED_COLOR_NORMAL);

    // 设置进入支付界面
    g_fruitVendingData.oledshowpage = OLED_SHOW_PAGE_PAYMENT;

    OLED_ShowFrame();
}

static void UI_pay_payment(void)
{
    OLED_NewFrame();

    // 显示标题
    OLED_PrintString(48, 0, "支付", &font16x16, OLED_COLOR_NORMAL);

    // 显示支付金额
    char amountStr[32];
    sprintf(amountStr, "金额: %d.%02d元",
            g_fruitVendingData.TransactionAmount / 100,
            g_fruitVendingData.TransactionAmount % 100);
    OLED_PrintString(0, 16, amountStr, &font16x16, OLED_COLOR_NORMAL);

    // 等待刷卡
    OLED_PrintString(0, 32, "请刷卡", &font16x16, OLED_COLOR_NORMAL);

    while (1)
    {
        if (g_fruitVendingData.PaymentSuccessFlag == 1)
        {
            UIManager_SwitchScreen(SCREEN_PAY_SUCCESS);
            break;
        }
        else if (g_fruitVendingData.PaymentErrorCode != 0)
        {
            UIManager_SwitchScreen(SCREEN_PAY_FAILURE);
            break;
        }
    }

    OLED_ShowFrame();
}

static void UI_pay_success(void)
{
    OLED_NewFrame();

    // 显示支付成功图标（进一步压缩的√）
    OLED_DrawLine(52, 8, 56, 12, OLED_COLOR_NORMAL);
    OLED_DrawLine(56, 12, 72, 4, OLED_COLOR_NORMAL);

    // 显示支付成功信息
    OLED_PrintString(32, 16, "支付成功", &font16x16, OLED_COLOR_NORMAL);

    // 显示交易金额
    char amountStr[32];
    sprintf(amountStr, "金额: %d.%02d元",
            g_fruitVendingData.TransactionAmount / 100,
            g_fruitVendingData.TransactionAmount % 100);
    OLED_PrintString(16, 32, amountStr, &font16x16, OLED_COLOR_NORMAL);

    // 显示卡内余额
    char balanceStr[32];
    sprintf(balanceStr, "卡内余额: %d.%02d元",
            g_fruitVendingData.CurrentUserCard_Money / 100,
            g_fruitVendingData.CurrentUserCard_Money % 100);
    OLED_PrintString(16, 48, balanceStr, &font16x16, OLED_COLOR_NORMAL);

    OLED_ShowFrame();
}

static void UI_pay_failure(void)
{
    OLED_NewFrame();

    // 显示支付失败图标（压缩的×，y坐标在0-16范围内）
    OLED_DrawLine(56, 4, 64, 12, OLED_COLOR_NORMAL);
    OLED_DrawLine(56, 12, 64, 4, OLED_COLOR_NORMAL);

    // 显示支付失败信息
    OLED_PrintString(32, 16, "支付失败", &font16x16, OLED_COLOR_NORMAL);

    // 显示错误代码
    char errorStr[32];
    sprintf(errorStr, "错误码: %d", g_fruitVendingData.PaymentErrorCode);
    OLED_PrintString(16, 32, errorStr, &font16x16, OLED_COLOR_NORMAL);


    // 显示错误原因
    if (g_fruitVendingData.PaymentErrorCode == 1)
    {
        /* 未检测到卡片 */
        // 等待
        
    }
    else if (g_fruitVendingData.PaymentErrorCode == 2)
    {
        OLED_PrintString(16, 48, "余额不足", &font16x16, OLED_COLOR_NORMAL);
    }
    else if (g_fruitVendingData.PaymentErrorCode == 3)
    {
        OLED_PrintString(16, 48, "卡号错误", &font16x16, OLED_COLOR_NORMAL);
    }

    OLED_ShowFrame();
}

static void UI_select_fruit(void)
{
    OLED_NewFrame();

    OLED_PrintString(16, 0, "选择水果", &font16x16, OLED_COLOR_NORMAL);

    // 根据当前活动水果的ID，显示水果名称
    char fruit_name[16];
    sprintf(fruit_name, "水果: %s", FruitName[g_fruitVendingData.ActionFruit]);
    OLED_PrintString(16, 16, fruit_name, &font16x16, OLED_COLOR_NORMAL);
    
    OLED_ShowFrame();
}

static void UI_hx711_set_calibration(void)
{
    OLED_NewFrame();

    // 显示标题
    OLED_PrintString(16, 0, "秤重校准", &font16x16, OLED_COLOR_NORMAL);

    // 显示当前值
    char valueStr[32];
    sprintf(valueStr, "当前值: %d", g_fruitVendingData.CurrentFruit_1_weight);
    OLED_PrintString(0, 16, valueStr, &font16x16, OLED_COLOR_NORMAL);

    // 显示操作提示
    OLED_PrintString(0, 32, "请放置标准重物", &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(0, 48, "按确认键保存", &font16x16, OLED_COLOR_NORMAL);

    OLED_ShowFrame();
}

static void UI_net_connect(void)
{
    OLED_NewFrame();

    // 显示网络连接状态
    OLED_PrintString(16, 16, "网络已连接", &font16x16, OLED_COLOR_NORMAL);

    // 显示IP地址
    char ipStr[32];
    sprintf(ipStr, "IP: %s", g_fruitVendingData.esp8266.ip);
    OLED_PrintString(0, 32, ipStr, &font16x16, OLED_COLOR_NORMAL);

    OLED_ShowFrame();
}

static void UI_net_disconnect(void)
{
    OLED_NewFrame();

    // 显示网络断开状态
    OLED_PrintString(16, 16, "网络已断开", &font16x16, OLED_COLOR_NORMAL);

    // 显示重连提示
    OLED_PrintString(0, 32, "正在尝试重连...", &font16x16, OLED_COLOR_NORMAL);

    OLED_ShowFrame();
}
/* 用户卡相关界面 */
static void UI_card_input(void);

static void UI_card_input_success(void);

static void UI_card_input_failed(void);

static void UI_card_recharge(void);

static void UI_card_balance(void);

/* 系统相关界面 */
static void UI_system_error(void);

static void UI_system_info(void);

static void UI_card_input(void)
{
    OLED_NewFrame();

    // 显示标题
    OLED_PrintString(16, 0, "卡片录入", &font16x16, OLED_COLOR_NORMAL);

    // 显示提示信息
    OLED_PrintString(0, 16, "请将新卡片", &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(0, 32, "放在读卡区", &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(0, 48, "长按取消", &font16x16, OLED_COLOR_NORMAL);

    OLED_ShowFrame();
}

static void UI_card_input_success(void)
{
    OLED_NewFrame();

    // 显示成功图标（√）
    OLED_DrawLine(52, 8, 56, 12, OLED_COLOR_NORMAL);
    OLED_DrawLine(56, 12, 72, 4, OLED_COLOR_NORMAL);

    // 显示成功信息
    OLED_PrintString(16, 16, "录入成功", &font16x16, OLED_COLOR_NORMAL);

    // 显示卡号
    char cardIdStr[32];
    sprintf(cardIdStr, "卡号:%02X%02X%02X%02X", 
            g_fruitVendingData.currentUser.user_card_id[0],
            g_fruitVendingData.currentUser.user_card_id[1],
            g_fruitVendingData.currentUser.user_card_id[2],
            g_fruitVendingData.currentUser.user_card_id[3]);
    OLED_PrintString(0, 32, cardIdStr, &font16x16, OLED_COLOR_NORMAL);

    OLED_ShowFrame();
}

static void UI_card_input_failed(void)
{
    OLED_NewFrame();

    // 显示失败图标（×）
    OLED_DrawLine(56, 4, 64, 12, OLED_COLOR_NORMAL);
    OLED_DrawLine(56, 12, 64, 4, OLED_COLOR_NORMAL);

    // 显示失败信息
    OLED_PrintString(16, 16, "录入失败", &font16x16, OLED_COLOR_NORMAL);

    // 显示错误原因
    if(g_fruitVendingData.cardInputErrorCode == ERROR_CARD_ALREADY_REGISTERED) {
        OLED_PrintString(0, 32, "卡片已注册", &font16x16, OLED_COLOR_NORMAL);
    } else if(g_fruitVendingData.cardInputErrorCode == ERROR_CARD_TABLE_FULL) {
        OLED_PrintString(0, 32, "用户表已满", &font16x16, OLED_COLOR_NORMAL);
    }

    OLED_ShowFrame();
}

static void UI_card_recharge(void)
{
    OLED_NewFrame();

    // 显示标题
    OLED_PrintString(16, 0, "卡片充值", &font16x16, OLED_COLOR_NORMAL);

    // 显示当前余额
    char balanceStr[32];
    sprintf(balanceStr, "当前余额:%d.%02d元", 
            g_fruitVendingData.currentUserCardMoney / 100,
            g_fruitVendingData.currentUserCardMoney % 100);
    OLED_PrintString(0, 16, balanceStr, &font16x16, OLED_COLOR_NORMAL);

    // 显示充值金额
    char rechargeStr[32];
    sprintf(rechargeStr, "充值金额:%d.%02d元",
            g_fruitVendingData.rechargeAmount / 100,
            g_fruitVendingData.rechargeAmount % 100);
    OLED_PrintString(0, 32, rechargeStr, &font16x16, OLED_COLOR_NORMAL);

    // 显示操作提示
    OLED_PrintString(0, 48, "请确认充值", &font16x16, OLED_COLOR_NORMAL);

    OLED_ShowFrame();
}

static void UI_card_balance(void)
{
    OLED_NewFrame();

    // 显示标题
    OLED_PrintString(16, 0, "余额查询", &font16x16, OLED_COLOR_NORMAL);

    // 显示卡号
    char cardIdStr[32];
    sprintf(cardIdStr, "卡号:%02X%02X%02X%02X", 
            g_fruitVendingData.currentUser.user_card_id[0],
            g_fruitVendingData.currentUser.user_card_id[1],
            g_fruitVendingData.currentUser.user_card_id[2],
            g_fruitVendingData.currentUser.user_card_id[3]);
    OLED_PrintString(0, 16, cardIdStr, &font16x16, OLED_COLOR_NORMAL);

    // 显示余额
    char balanceStr[32];
    sprintf(balanceStr, "余额:%d.%02d元",
            g_fruitVendingData.currentUserCardMoney / 100,
            g_fruitVendingData.currentUserCardMoney % 100);
    OLED_PrintString(0, 32, balanceStr, &font16x16, OLED_COLOR_NORMAL);

    OLED_ShowFrame();
}

/* 系统相关界面 */
static void UI_system_error(void)
{
    OLED_NewFrame();

    // 显示错误图标
    OLED_DrawLine(56, 4, 64, 12, OLED_COLOR_NORMAL);
    OLED_DrawLine(56, 12, 64, 4, OLED_COLOR_NORMAL);

    // 显示错误信息
    OLED_PrintString(16, 0, "系统错误", &font16x16, OLED_COLOR_NORMAL);

    // 显示错误代码和描述
    char errorStr[32];
    sprintf(errorStr, "错误码:%d", g_fruitVendingData.systemErrorCode);
    OLED_PrintString(0, 16, errorStr, &font16x16, OLED_COLOR_NORMAL);

    // 根据错误代码显示具体错误信息
    switch(g_fruitVendingData.systemErrorCode) {
        case 1:
            OLED_PrintString(0, 32, "秤重传感器", &font16x16, OLED_COLOR_NORMAL);
            OLED_PrintString(0, 48, "初始化失败", &font16x16, OLED_COLOR_NORMAL);
            break;
        case 2:
            OLED_PrintString(0, 32, "温湿度传感器", &font16x16, OLED_COLOR_NORMAL);
            OLED_PrintString(0, 48, "初始化失败", &font16x16, OLED_COLOR_NORMAL);
            break;
        case 3:
            OLED_PrintString(0, 32, "EEPROM存储器", &font16x16, OLED_COLOR_NORMAL);
            OLED_PrintString(0, 48, "读写错误", &font16x16, OLED_COLOR_NORMAL);
            break;
        default:
            OLED_PrintString(0, 32, "未知错误", &font16x16, OLED_COLOR_NORMAL);
            break;
    }

    OLED_ShowFrame();
}

static void UI_system_info(void)
{
    OLED_NewFrame();

    // 显示标题
    OLED_PrintString(0, 0, "系统信息", &font16x16, OLED_COLOR_NORMAL);

    // 显示版本信息
    OLED_PrintString(0, 16, "版本:V1.0", &font16x16, OLED_COLOR_NORMAL);

    // 显示运行时间
    char timeStr[32];
    uint32_t hours = HAL_GetTick() / 3600000;
    uint32_t minutes = (HAL_GetTick() % 3600000) / 60000;
    sprintf(timeStr, "运行:%02dh%02dm", (int)hours, (int)minutes);
    OLED_PrintString(0, 32, timeStr, &font16x16, OLED_COLOR_NORMAL);

    // 显示存储状态
    char storageStr[32];
    sprintf(storageStr, "存储:%d/%d", g_fruitVendingData.userTableIndex, USER_NUM_MAX);
    OLED_PrintString(0, 48, storageStr, &font16x16, OLED_COLOR_NORMAL);

    OLED_ShowFrame();
}



