#include "flasher.h"
#include "./ui_flasher.h"

Flasher::Flasher(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Flasher)
{
    ui->setupUi(this);
}

Flasher::~Flasher()
{
    delete ui;
}

