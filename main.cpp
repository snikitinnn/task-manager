
#include <QApplication>
#include <QtWidgets>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QPainter>
#include <QFileDialog>


class StrikeThroughDelegate : public QStyledItemDelegate {
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QStyleOptionViewItem myOption = option;
        QString text = index.data(Qt::DisplayRole).toString();

        if (index.data(Qt::CheckStateRole).toInt() == Qt::Checked) {
            myOption.text = text;
            QStyledItemDelegate::paint(painter, myOption, index);

            QFontMetrics fm = painter->fontMetrics();
            QRect textRect = fm.boundingRect(option.rect, Qt::AlignVCenter | Qt::AlignLeft, text);
            int x = textRect.left();
            int y = textRect.center().y();
            painter->drawLine(x, y, x + textRect.width(), y);
        } else {
            QStyledItemDelegate::paint(painter, myOption, index);
        }
    }
};


class TaskList : public QWidget {
    Q_OBJECT

public:
    TaskList(QWidget *parent = nullptr) : QWidget(parent) {
        settings = new QSettings("MyCompany", "TaskList", this);

        lineEdit = new QLineEdit(this);
        addButton = new QPushButton("Добавить", this);
        listWidget = new QListWidget(this);
        removeButton = new QPushButton("Удалить", this);
        loadButton = new QPushButton("Загрузить", this);
        saveButton = new QPushButton("Сохранить", this);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        QHBoxLayout *inputLayout = new QHBoxLayout();
        inputLayout->addWidget(lineEdit);
        inputLayout->addWidget(addButton);
        mainLayout->addLayout(inputLayout);
        mainLayout->addWidget(listWidget);
        mainLayout->addWidget(removeButton);
        mainLayout->addWidget(loadButton);
        mainLayout->addWidget(saveButton);

        connect(addButton, &QPushButton::clicked, this, &TaskList::addTask);
        connect(removeButton, &QPushButton::clicked, this, &TaskList::removeTask);
        connect(listWidget, &QListWidget::itemChanged, this, &TaskList::itemChanged);
        connect(loadButton, &QPushButton::clicked, this, &TaskList::loadTasksFromFile);
        connect(saveButton, &QPushButton::clicked, this, &TaskList::saveTasksToFile);
        listWidget->setItemDelegate(new StrikeThroughDelegate);

        restoreGeometry(settings->value("geometry").toByteArray());
    }

    ~TaskList() {
        settings->setValue("geometry", saveGeometry());
        delete settings;
    }


private slots:
    void addTask() {
        QString task = lineEdit->text().trimmed();
        if (task.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Поле ввода пустое!");
            return;
        }

        QListWidgetItem *item = new QListWidgetItem(task);
        listWidget->addItem(item);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        lineEdit->clear();
        listWidget->setCurrentItem(item);
        listWidget->repaint();
    }

    void removeTask() {
        QList<QListWidgetItem*> selectedItems = listWidget->findItems("", Qt::MatchContains);
        for (QListWidgetItem* item : selectedItems) {
            if (item->checkState() == Qt::Checked) {
                delete item;
            }
        }

        if (selectedItems.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Пожалуйста, выберите задачи для удаления!");
        }
    }

    void itemChanged(QListWidgetItem *item) {
        listWidget->update();
    }

    void loadTasksFromFile() {
        QString fileName = QFileDialog::getOpenFileName(this, "Загрузить список задач", QDir::homePath(), "Text Files (*.txt)");
        if (!fileName.isEmpty()) {
            loadTasks(fileName);
        }
    }

    void saveTasksToFile() {
        QString fileName = QFileDialog::getSaveFileName(this, "Сохранить список задач", QDir::homePath(), "Text Files (*.txt)");
        if (!fileName.isEmpty()) {
            saveTasks(fileName);
        }
    }

private:
    void loadTasks(const QString& fileName) {
        QFile file(fileName);
        if (file.open(QFile::ReadOnly)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine();
                QListWidgetItem *item = new QListWidgetItem(line);
                listWidget->addItem(item);
                item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                if (line.startsWith("*")) {
                    item->setCheckState(Qt::Checked);
                    line.remove(0, 1);
                    item->setText(line);
                } else {
                    item->setCheckState(Qt::Unchecked);
                }
            }
            file.close();
        }
    }

    void saveTasks(const QString& fileName) {
        QFile file(fileName);
        if (file.open(QFile::WriteOnly | QFile::Text)) {
            QTextStream out(&file);
            for (int i = 0; i < listWidget->count(); ++i) {
                QListWidgetItem *item = listWidget->item(i);
                QString text = item->text();
                if (item->checkState() == Qt::Checked) {
                    out << "*" << text << "\n";
                } else {
                    out << text << "\n";
                }
            }
            file.close();
        }
    }

    QLineEdit *lineEdit;
    QPushButton *addButton;
    QListWidget *listWidget;
    QPushButton *removeButton;
    QPushButton *loadButton;
    QPushButton *saveButton;
    QSettings *settings;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    TaskList taskList;
    taskList.show();
    return app.exec();
}

#include "main.moc"
