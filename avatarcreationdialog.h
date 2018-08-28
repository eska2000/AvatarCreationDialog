#ifndef AVATARCREATIONDIALOG_H
#define AVATARCREATIONDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QScrollArea>

#define AVATAR_WIDTH 300
#define AVATAR_HEIGHT 400

//#define SAVE_TO_FILE

class AvatarCreationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AvatarCreationDialog(QWidget *parent = nullptr);

private slots:
    void on_buttonBox_accepted();
    void dialogAbort();

signals:
    void result(const QImage &avatar);

private:
    void openFileDialog() override;
    void setImage();
    void loadFile(const QString &fileName);

    QLabel *imageLabel;
    QImage image;
    QScrollArea *scrollArea;
    double scaleFactor = 1.0;

    void keyPressEvent(QKeyEvent *event) override;
    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);
    QRect visibleImageRect();
};

#endif // AVATARCREATIONDIALOG_H
