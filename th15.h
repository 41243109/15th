#ifndef TH15_H
#define TH15_H

#include <QMainWindow>
#include <QVector>
#include <QDate>
#include <QLineEdit>
#include <QTableWidgetItem>
#include <QTableWidget>
#include <QDateEdit>
#include <QLabel>
#include <QComboBox>

struct Record {
    QDate date;
    QString category;
    double amount;
    QString note;
};

QT_BEGIN_NAMESPACE
namespace Ui {
class th15;
}
QT_END_NAMESPACE

class th15 : public QMainWindow
{
    Q_OBJECT

public:
    th15(QWidget *parent = nullptr);
    ~th15();

private slots:
    void loadRecordsFromFile();  //載入文件紀錄
    void saveRecordsToFile();  // 保存文件記錄
    void updateUI();  //更新UI
    void updateBalance();  //餘額更新
    void queryRecords();  //查詢紀錄
    void addIncomeRecord();  //新增收入
    void addExpenseRecord();  //新增支出
private:
    QVector<Record> records;  //紀錄列表
    double totalIncome;  //總收入
    double totalExpense;  //總支出

    QLineEdit *categoryInput;  //輸入類別框
    QLineEdit *amountInput;  //輸入金額框
    QLineEdit *noteInput;  //輸入備註框
    QDateEdit *dateInput;  //選擇日期

    QLabel *totalIncomeLabel;  //顯示總收入
    QLabel *totalExpenseLabel;  //顯示總支出
    QLabel *balanceLabel;  //顯示餘額

    QTableWidget *queryResults;  //結果表格

    QDateEdit *startDate;  //開始日期
    QDateEdit *endDate;  //結束日期

    QComboBox *timeRangeComboBox;  //周/月下單列表
    Ui::th15 *ui;
};
#endif // TH15_H
