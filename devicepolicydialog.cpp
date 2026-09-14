#include "devicepolicydialog.h"
#include "ui_devicepolicydialog.h"

DevicePolicyDialog::DevicePolicyDialog(MySqlLite *sqltie,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DevicePolicyDialog),
    mysqltie(sqltie)
{
    ui->setupUi(this);
    setWindowTitle("选择用户");

//    setStyleSheet("QDialog { "
//                  "   border-image: url(:/image/dm8318.png) stretch; "
//                  "   background-color: transparent; "
//                  "} "
//                  "QWidget { "
//                  "   background: transparent; "
//                  "}");
    connect(this,&DevicePolicyDialog::getUserNoList,mysqltie,&MySqlLite::getUserListCAR_DEPOT);
    connect(mysqltie,&MySqlLite::UserListCAR_DEPOT,this,&DevicePolicyDialog::setListWidgetData);

    connect(ui->pushButtonsearchUserNo, &QPushButton::clicked, this, &DevicePolicyDialog::searchUserNo);
    connect(ui->pushButtonok, &QPushButton::clicked, this, &DevicePolicyDialog::accept);
    emit getUserNoList();

    // Populate the listWidget with UserNo data (example data)
//    QStringList userNos = {"UserNo1", "UserNo2", "UserNo3", "UserNo4"};
    ui->listWidget->addItems(myuserNos);
    ui->listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
}

DevicePolicyDialog::~DevicePolicyDialog() {
    delete ui;
}

QString DevicePolicyDialog::getDevNo() const {
    return devNo;  // Return the stored devNo
}

QString DevicePolicyDialog::getPolNo() const {
    return polNo;  // Return the stored polNo (selected UserNo)
}

void DevicePolicyDialog::searchUserNo() {
    QString searchText = ui->lineEditPelNo->text();
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem *item = ui->listWidget->item(i);
        item->setHidden(!item->text().contains(searchText, Qt::CaseInsensitive));
    }
}

void DevicePolicyDialog::accept() {
    if (!ui->lineEditdevNo->text().isEmpty() && ui->listWidget->currentItem()) {

        devNo = ui->lineEditdevNo->text().isEmpty() ? "000000" : ui->lineEditdevNo->text();

        // 检查 listWidget 中是否有选中项，如果没有则设置为 "000000"
        if (ui->listWidget->currentItem()) {
            polNo = ui->listWidget->currentItem()->text();
        } else {
            polNo = "000000";
        }
        QDialog::accept();
    }
}
void DevicePolicyDialog::reject() {
    // 如果需要，可以在这里添加额外的逻辑
    QDialog::reject();
}
void DevicePolicyDialog::setListWidgetData(const QStringList &userNos)
{
    myuserNos = userNos;
}

