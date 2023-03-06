#ifndef FLASHER_H
#define FLASHER_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class Flasher; }
QT_END_NAMESPACE

class Flasher : public QMainWindow
{
    Q_OBJECT

public:
    Flasher(QWidget *parent = nullptr);
    ~Flasher();

private:
    Ui::Flasher *ui;
};
#endif // FLASHER_H
