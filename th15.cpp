#include "th15.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QDateEdit>
#include <QComboBox>
#include <QTableWidget>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QDate>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QJsonDocument>

th15::th15(QWidget *parent) : QMainWindow(parent), totalIncome(0), totalExpense(0) {
    setWindowTitle("記帳小幫手");  //窗口標題

    //主界面容器
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    //記錄區域
    QLabel *recordLabel = new QLabel("記錄區域", this);
    mainLayout->addWidget(recordLabel);

    //創建日期輸入控件
    dateInput = new QDateEdit(QDate::currentDate(), this);
    dateInput->setCalendarPopup(true);  //顯示日曆選擇器
    mainLayout->addWidget(dateInput);  //添加到布局中

    //類別輸入框
    QLabel *categoryLabel = new QLabel("類別", this);
    mainLayout->addWidget(categoryLabel);

    categoryInput = new QLineEdit(this);  // 初始化 categoryInput
    categoryInput->setPlaceholderText("請輸入類別（例如：餐費、交通費等）");
    mainLayout->addWidget(categoryInput);

    // 金額輸入框
    amountInput = new QLineEdit(this);
    amountInput->setPlaceholderText("輸入金額");
    mainLayout->addWidget(amountInput);

    // 備註欄位
    noteInput = new QLineEdit(this);
    noteInput->setPlaceholderText("備註");
    mainLayout->addWidget(noteInput);

    // 新增收入和新增支出按鈕
    QPushButton *addIncomeButton = new QPushButton("新增收入", this);
    mainLayout->addWidget(addIncomeButton);

    QPushButton *addExpenseButton = new QPushButton("新增支出", this);
    mainLayout->addWidget(addExpenseButton);

    // 連接按鈕點擊事件
    connect(addIncomeButton, &QPushButton::clicked, this, &th15::addIncomeRecord);
    connect(addExpenseButton, &QPushButton::clicked, this, &th15::addExpenseRecord);

    // 設置週/月選擇區域
    QLabel *timeRangeLabel = new QLabel("選擇週/月範圍", this);
    mainLayout->addWidget(timeRangeLabel);

    timeRangeComboBox = new QComboBox(this);
    timeRangeComboBox->addItems({"本週", "本月"}); // 設置選擇週/月的選項
    mainLayout->addWidget(timeRangeComboBox);
    connect(timeRangeComboBox, &QComboBox::currentIndexChanged, this, &th15::updateBalance);

    // 統計區域
    QLabel *statLabel = new QLabel("統計區域", this);
    mainLayout->addWidget(statLabel);

    totalIncomeLabel = new QLabel("總收入：0", this);
    totalExpenseLabel = new QLabel("總支出：0", this);
    balanceLabel = new QLabel("餘額：0", this);  // 新增的餘額標籤
    mainLayout->addWidget(totalIncomeLabel);
    mainLayout->addWidget(totalExpenseLabel);
    mainLayout->addWidget(balanceLabel);

    // 查詢區域
    QLabel *queryLabel = new QLabel("查詢區域", this);
    mainLayout->addWidget(queryLabel);

    startDate = new QDateEdit(QDate::currentDate(), this);
    endDate = new QDateEdit(QDate::currentDate(), this);
    startDate->setCalendarPopup(true); // 開啟日曆彈出框
    mainLayout->addWidget(startDate);
    endDate->setCalendarPopup(true);   // 開啟日曆彈出框
    mainLayout->addWidget(endDate);

    QPushButton *queryButton = new QPushButton("查詢", this);
    mainLayout->addWidget(queryButton);

    // 查詢按鈕連接查詢事件
    connect(queryButton, &QPushButton::clicked, this, &th15::queryRecords);

    queryResults = new QTableWidget(this);
    queryResults->setColumnCount(4); // 設定列數為4
    queryResults->setHorizontalHeaderLabels({"日期", "類別", "金額", "備註"}); // 設置表格標頭

    // 使表格可編輯
    queryResults->setEditTriggers(QAbstractItemView::DoubleClicked);

    mainLayout->addWidget(queryResults);

    setCentralWidget(centralWidget);// 設置主窗口

    updateBalance();// 更新餘額

    loadRecordsFromFile();// 加載記錄文件
}

th15::~th15() {
    saveRecordsToFile(); // 程式退出時保存記錄
}

void th15::saveRecordsToFile() {
    QFile file("records.json"); // 打開名為 "records.json" 的文件
    if (!file.open(QIODevice::WriteOnly)) { // 如果文件無法打開，顯示錯誤
        qDebug() << "無法打開文件保存記錄：" << file.errorString();
        return;
    }

    QJsonArray recordArray;
    // 遍歷所有記錄
    for (const Record &record : records) {
        QJsonObject recordObject;
        recordObject["date"] = record.date.toString("yyyy-MM-dd");
        recordObject["category"] = record.category;
        recordObject["amount"] = record.amount;
        recordObject["note"] = record.note;
        recordArray.append(recordObject); // 將記錄加入到 JSON 陣列中
    }

    QJsonDocument doc(recordArray); // 生成 JSON 文檔
    file.write(doc.toJson()); // 寫入文件
    file.close(); // 關閉文件

    qDebug() << "記錄已保存！"; // 輸出保存成功信息
}

// 從 JSON 文件加載記錄
void th15::loadRecordsFromFile() {
    QFile file("records.json"); // 打開 "records.json" 文件
    if (!file.open(QIODevice::ReadOnly)) { // 如果文件無法打開，顯示錯誤
        qDebug() << "無法打開文件加載記錄：" << file.errorString();
        return;
    }

    QByteArray data = file.readAll(); // 讀取文件內容
    QJsonDocument doc = QJsonDocument::fromJson(data); // 解析 JSON 文檔
    file.close(); // 關閉文件

    if (!doc.isArray()) { // 如果文檔不是 JSON 陣列格式，顯示錯誤
        qDebug() << "文件格式錯誤！";
        return;
    }

    QJsonArray recordArray = doc.array(); // 獲取 JSON 陣列
    // 遍歷 JSON 陣列中的每一項
    for (const QJsonValue &value : recordArray) {
        QJsonObject recordObject = value.toObject();
        Record record;
        record.date = QDate::fromString(recordObject["date"].toString(), "yyyy-MM-dd");
        record.category = recordObject["category"].toString();
        record.amount = recordObject["amount"].toDouble();
        record.note = recordObject["note"].toString();
        records.append(record); // 將記錄加入到 records 容器中
    }

    qDebug() << "記錄加載成功！"; // 輸出加載成功信息

    // 更新 UI 顯示
    updateUI();
}

void th15::updateUI() {
    queryResults->setRowCount(0); // 清空表格
    totalIncome = 0; // 初始化總收入
    totalExpense = 0; // 初始化總支出

    // 遍歷所有記錄，更新表格和統計數據
    for (const Record &record : records) {
        int row = queryResults->rowCount();
        queryResults->insertRow(row); // 插入新行
        queryResults->setItem(row, 0, new QTableWidgetItem(record.date.toString("yyyy-MM-dd")));
        queryResults->setItem(row, 1, new QTableWidgetItem(record.category));
        queryResults->setItem(row, 2, new QTableWidgetItem(QString::number(record.amount)));
        queryResults->setItem(row, 3, new QTableWidgetItem(record.note));

        // 根據金額更新總收入和總支出
        if (record.amount > 0) {
            totalIncome += record.amount;
        } else {
            totalExpense += record.amount;
        }
    }

    // 更新 UI 顯示的收入、支出和餘額
    totalIncomeLabel->setText(QString("總收入：%1").arg(totalIncome));
    totalExpenseLabel->setText(QString("總支出：%1").arg(totalExpense));
    balanceLabel->setText(QString("餘額：%1").arg(totalIncome - totalExpense));
}

// 新增收入記錄A
void th15::addIncomeRecord() {
    QString category = categoryInput->text();
    bool ok;
    double amount = amountInput->text().toDouble(&ok); // 讀取並檢查金額
    QString note = noteInput->text();
    QDate date = dateInput->date();  // 獲取日期

    if (!ok || amount <= 0) { // 如果金額無效，顯示錯誤訊息
        QMessageBox::warning(this, "輸入錯誤", "請確保金額為有效數字，並且大於 0！");
        return;
    }

    // 如果金額大於等於 1000，彈出提示
    if (amount >= 1000) {
        QMessageBox::warning(this, "水喔", "有賺欸!美麥喔!");
    }

    Record newRecord = {date, category, amount, note}; // 創建新的收入記錄
    records.append(newRecord); // 將記錄加入到記錄容器中

    // 更新總收入
    totalIncome += amount;

    // 更新顯示
    totalIncomeLabel->setText(QString("總收入：%1").arg(totalIncome));
    totalExpenseLabel->setText(QString("總支出：%1").arg(totalExpense));
    balanceLabel->setText(QString("餘額：%1").arg(totalIncome - totalExpense));  // 計算餘額

    // 更新表格顯示
    int row = queryResults->rowCount();
    queryResults->insertRow(row);
    queryResults->setItem(row, 0, new QTableWidgetItem(newRecord.date.toString("yyyy-MM-dd")));
    queryResults->setItem(row, 1, new QTableWidgetItem(newRecord.category));
    queryResults->setItem(row, 2, new QTableWidgetItem(QString::number(newRecord.amount)));
    queryResults->setItem(row, 3, new QTableWidgetItem(newRecord.note));

    QMessageBox::information(this, "新增成功", "收入記錄已成功新增！"); // 顯示成功訊息

    amountInput->clear(); // 清空金額輸入框
    noteInput->clear();   // 清空備註輸入框
}

// 新增支出記錄
void th15::addExpenseRecord() {
    QString category = categoryInput->text();
    bool ok;
    double amount = amountInput->text().toDouble(&ok); // 讀取並檢查金額
    QString note = noteInput->text();
    QDate date = dateInput->date();  // 獲取日期

    if (!ok || amount <= 0) { // 如果金額無效，顯示錯誤訊息
        QMessageBox::warning(this, "輸入錯誤", "請確保金額為有效數字，並且大於 0！");
        return;
    }

     if (amount >= 1000) {
        QMessageBox::warning(this, "喂喂喂", "買買買!你的錢是大風颳來的喔!");
    }

    // 支出金額為負數
    Record newRecord = {date, category, -amount, note}; // 創建支出記錄
    records.append(newRecord); // 將記錄加入到記錄容器中

    // 更新總支出
    totalExpense += amount;

    // 更新顯示
    totalIncomeLabel->setText(QString("總收入：%1").arg(totalIncome));
    totalExpenseLabel->setText(QString("總支出：%1").arg(totalExpense));
    balanceLabel->setText(QString("餘額：%1").arg(totalIncome - totalExpense));  // 計算餘額

    // 更新表格顯示
    int row = queryResults->rowCount();
    queryResults->insertRow(row);
    queryResults->setItem(row, 0, new QTableWidgetItem(newRecord.date.toString("yyyy-MM-dd")));
    queryResults->setItem(row, 1, new QTableWidgetItem(newRecord.category));
    queryResults->setItem(row, 2, new QTableWidgetItem(QString::number(newRecord.amount)));
    queryResults->setItem(row, 3, new QTableWidgetItem(newRecord.note));

    QMessageBox::information(this, "新增成功", "支出記錄已成功新增！"); // 顯示成功訊息

    amountInput->clear(); // 清空金額輸入框
    noteInput->clear();   // 清空備註輸入框
}

// 更新餘額顯示
void th15::updateBalance() {
    // 根據選擇的時間範圍進行篩選
    QDate currentDate = QDate::currentDate();
    QDate startDateValue;
    QDate endDateValue;

    if (timeRangeComboBox->currentText() == "本週") {
        startDateValue = currentDate.addDays(-currentDate.dayOfWeek() + 1); // 本周一
        endDateValue = startDateValue.addDays(6); // 本周日
    } else {
        startDateValue = currentDate.addDays(-currentDate.day()); // 本月1號
        endDateValue = currentDate; // 本月底
    }

    double filteredIncome = 0;
    double filteredExpense = 0;

    // 遍歷所有記錄，根據時間範圍進行篩選
    for (const Record &record : records) {
        if (record.date >= startDateValue && record.date <= endDateValue) {
            if (record.amount > 0) {
                filteredIncome += record.amount;
            } else {
                filteredExpense += record.amount;
            }
        }
    }

    // 更新 UI 顯示的收入、支出和餘額
    totalIncomeLabel->setText(QString("總收入：%1").arg(filteredIncome));
    totalExpenseLabel->setText(QString("總支出：%1").arg(filteredExpense));
    balanceLabel->setText(QString("餘額：%1").arg(filteredIncome - filteredExpense));
}

// 查詢記錄
void th15::queryRecords() {
    QDate start = startDate->date();
    QDate end = endDate->date();

    queryResults->setRowCount(0); // 清空表格

    // 遍歷所有記錄，根據日期範圍進行篩選
    for (const Record &record : records) {
        if (record.date >= start && record.date <= end) {
            int row = queryResults->rowCount(); //回傳目前表格的總行數
            queryResults->insertRow(row);       //在表格中插入一行新的資料列
            queryResults->setItem(row, 0, new QTableWidgetItem(record.date.toString("yyyy-MM-dd")));
            queryResults->setItem(row, 1, new QTableWidgetItem(record.category));
            queryResults->setItem(row, 2, new QTableWidgetItem(QString::number(record.amount)));
            queryResults->setItem(row, 3, new QTableWidgetItem(record.note));
        }
    }
}
