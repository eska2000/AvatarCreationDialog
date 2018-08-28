#include <QFileDialog>
#include <QStandardPaths>
#include <QImageReader>
#include <QMessageBox>
//#include <QDebug>
#include <QScrollBar>
#include <QKeyEvent>

#include "avatarcreationdialog.h"
#include "ui_avatarcreationdialog.h"

AvatarCreationDialog::AvatarCreationDialog(QWidget *parent) :
    QDialog(parent)
{
    openFileDialog();
}

void AvatarCreationDialog::openFileDialog()
{
    QFileDialog dialog(nullptr, tr("Выберите фотогрфию"));
    dialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).last());

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = QImageReader::supportedMimeTypes();
    foreach (const QByteArray &mimeTypeName, supportedMimeTypes)
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");

    if (dialog.exec() == QDialog::Accepted) {
        loadFile(dialog.selectedFiles().first());
        return;
    }

    dialogAbort();
}

void AvatarCreationDialog::dialogAbort()
{
    QImage image;
    emit result(image);
    this->close();
}

void AvatarCreationDialog::loadFile(const QString &fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    image = reader.read();
    if (image.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Невозможно загрузить файл %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        dialogAbort();
    }
    else {
        setImage();
    }
}

void AvatarCreationDialog::setImage()
{
    this->setWindowTitle("Диалог редактирования аватара");

    QVBoxLayout *vBoxLayout = new QVBoxLayout(this);
    vBoxLayout->setMargin(11);
    vBoxLayout->setSpacing(6);

    scrollArea = new QScrollArea();
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    scrollArea->setMinimumSize(AVATAR_WIDTH, AVATAR_HEIGHT);

    imageLabel = new QLabel();
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    scrollArea->setWidget(imageLabel);
    vBoxLayout->addWidget(scrollArea);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    vBoxLayout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &AvatarCreationDialog::on_buttonBox_accepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AvatarCreationDialog::dialogAbort);

    this->adjustSize();
    this->setMaximumSize(this->size());
    this->setMinimumSize(this->size());

    imageLabel->setPixmap(QPixmap::fromImage(image));
    imageLabel->adjustSize();

    show();
}

void AvatarCreationDialog::scaleImage(double factor)
{
    Q_ASSERT(imageLabel->pixmap());
    scaleFactor *= factor;
    imageLabel->resize(scaleFactor * imageLabel->pixmap()->size());

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);
}

void AvatarCreationDialog::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}

void AvatarCreationDialog::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Plus:
        scaleImage(1.25/1.1);
        break;
    case Qt::Key_Minus:
        scaleImage(0.8/1.1);
        break;
    case Qt::Key_Enter:
        on_buttonBox_accepted();
        break;
    case Qt::Key_Return:
        on_buttonBox_accepted();
        break;
    case Qt::Key_Escape:
        dialogAbort();
        break;
    default:
        break;
    }
}

QRect AvatarCreationDialog::visibleImageRect()
{
    return QRect(int(scrollArea->horizontalScrollBar()->value() / scaleFactor),
                 int(scrollArea->verticalScrollBar()->value() / scaleFactor),
                 int(scrollArea->maximumViewportSize().width() / scaleFactor),
                 int(scrollArea->maximumViewportSize().height() / scaleFactor));
}

void AvatarCreationDialog::on_buttonBox_accepted()
{

    QImage cropped = image.copy(visibleImageRect());
    cropped = cropped.scaled(scrollArea->minimumWidth(), scrollArea->minimumHeight(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    emit result(cropped);

#ifdef SAVE_TO_FILE
    QString dir = QFileDialog::getExistingDirectory(this);
    if (dir.isEmpty())
        return;
    cropped.save(dir+QDir::separator()+"test.jpg");
#endif

    this->close();
}
