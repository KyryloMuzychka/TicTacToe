#include "widget.h"
#include "ui_widget.h"
#include "stylehelper.h"
#include <QTabBar>
#include <QStyleOption>
#include <QDebug>
#include <QFontDatabase>
#include <QGridLayout>
#include <time.h>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    configurationTabWidget();
    addFonts();
    setInterfaceStyle();
    configurationGameAreaButtons();
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Widget::onComputerSlot);
}

Widget::~Widget()
{
    delete ui;
}

/* Применяем к Widget правила QSS  */
void Widget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);
}

/* Назначаем правила QSS для элементов интерфейса  */
void Widget::setInterfaceStyle()
{
    this->setStyleSheet(StyleHelper::getMainWidgetStyle());
    ui->startButton->setStyleSheet(StyleHelper::getStartButtonsStyle());
    ui->aboutButton->setStyleSheet(StyleHelper::getStartButtonsStyle());
    ui->leftButton->setStyleSheet(StyleHelper::getLeftButtonActiveStyle());
    ui->rightButton->setStyleSheet(StyleHelper::getRightButtonStyle());
    ui->tabWidget->setStyleSheet(StyleHelper::getTabWidgetStyle());
    ui->tab->setStyleSheet(StyleHelper::getTabStyle());
    ui->messageLabel->setStyleSheet(StyleHelper::getVictoryMessageStyle());
    ui->messageLabel->setText("Ходят крестики");
    setAreaButtonsStyle();
    ui->messageLabel->setText("");
    ui->messageLabel->setStyleSheet(StyleHelper::getNormalMessageStyle());
    ui->labelAbout->setStyleSheet(StyleHelper::getAboutTextStyle());
}

/* Переключаем выбор крестики-нолики  */
void Widget::changeButtonStatus(int num)
{
    if(num ==1){
        ui->leftButton->setStyleSheet(StyleHelper::getLeftButtonActiveStyle());
        ui->rightButton->setStyleSheet(StyleHelper::getRightButtonStyle());
    }
    else
    {
        ui->leftButton->setStyleSheet(StyleHelper::getLeftButtonStyle());
        ui->rightButton->setStyleSheet(StyleHelper::getRightButtonActiveStyle());
    }
}

/* Если игрок выбрал крестики  */
void Widget::on_leftButton_clicked()
{
    changeButtonStatus(1);
    player = 'X';
}

/* Если игрок выбрал нолики  */
void Widget::on_rightButton_clicked()
{
    changeButtonStatus(2);
    player = '0';
}


/* Скрываем заголовок, выставляем высоту и текущую вкладку tabWidget  */
void Widget::configurationTabWidget()
{
    ui->tabWidget->tabBar()->hide();
    ui->tabWidget->setMaximumHeight(320);
    ui->tabWidget->setCurrentIndex(0);
}

/* Добавляем шрифты из ресурсов в базу шрифтов  */
void Widget::addFonts()
{
    QFontDatabase::addApplicationFont(":/resourses/fonts/Roboto-Medium.ttf");
    int id = QFontDatabase::addApplicationFont(":/resourses/fonts/Roboto-MediumItalic.ttf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);
    qDebug() << family;
}

void Widget::changeButtonStyle(int row, int column, QString style)
{
    QGridLayout *grid = qobject_cast <QGridLayout*>(ui->tab->layout());
    QPushButton *btn = qobject_cast <QPushButton*>(grid->itemAtPosition(row,column)->widget()); // получаем указаель на кнопку
    btn ->setStyleSheet(style);
}

void Widget::configurationGameAreaButtons()
{
    QGridLayout *grid = qobject_cast <QGridLayout*>(ui->tab->layout());// получаем указатель на компоновщик
    for (int row = 0; row < 3; row++)
    {
        for (int col = 0; col < 3; col++)
        {
            QPushButton *btn = qobject_cast <QPushButton*>(grid->itemAtPosition(row, col)->widget());
            btn->setProperty("row", row);
            btn->setProperty("column", col);
            connect(btn, &QPushButton::clicked, this, &Widget::onGameAreaButtonClicked);
        }
    }
}

void Widget::setAreaButtonsStyle()
{
    for (int row = 0; row < 3; row++)
        for (int column = 0; column < 3; column++)
            changeButtonStyle(row, column, StyleHelper::getBlankButtonStyle());
}

void Widget::start()
{
    setAreaButtonsStyle();
    for(int i = 0; i < 3; i++)
        for(int j = 0; j < 3; j++)
            gameArea[i][j] = '-';
    progress = 0;
    gameStart = true;
    stop = false;
    if (player != 'X')
        computerInGame();
}

void Widget::lockPlayer()
{
    playerLocked = player == 'X' ? false : true;
}

void Widget::on_startButton_clicked()
{
    if (gameStart)
    {
        playerLocked = true;
        ui->startButton->setText("Играть");
        ui->startButton->setStyleSheet(StyleHelper::getStartButtonsStyle());
        ui->leftButton->setDisabled(false);
        ui->rightButton->setDisabled(false);
        gameStart = false;
        ui->messageLabel->setText("Проиграл");
        ui->messageLabel->setStyleSheet(StyleHelper::getLostMessageStyle());
    }
    else
    {
        ui->messageLabel->setText("Поехали!");
        ui->messageLabel->setStyleSheet(StyleHelper::getNormalMessageStyle());
        start();
        lockPlayer();
        ui->startButton->setText("Сдаюсь");
        ui->startButton->setStyleSheet(StyleHelper::getStartButtonGameStyle());
        ui->leftButton->setDisabled(true);
        ui->rightButton->setDisabled(true);
    }
}

void Widget::onGameAreaButtonClicked()
{
    if (!playerLocked)
    {
        QString style;
        QPushButton *btn = qobject_cast <QPushButton*>(sender());
        int row = btn->property("row").toInt();
        int column = btn ->property("column").toInt();
        if (gameArea[row][column] == '-')
        {
            if (player == 'X')
            {
                style = StyleHelper::getCrossNormalStyle();
                gameArea[row][column] = 'X';
            }
            else
            {
                style = StyleHelper::getZeroNormalStyle();
                gameArea[row][column] = '0';
            }
            changeButtonStyle(row, column, style);
            playerLocked = true;
            progress++;
            checkGameStop();
            endGame();
            computerInGame();
        }
    }
}

void Widget::computerInGame()
{
    if(stop)
        return;
    srand((unsigned)time(0));
    int index = rand() % 4;
    QStringList list = {"Мой ход", "Так так так...", "Хм... сложно...", "Дайте подумать"};
    ui->messageLabel->setText(list.at(index));
    timer->start(1000);
}

void Widget::checkGameStop()
{
    winner = '-';        
    char symbols[2] = {'X', '0'};
    for (int i = 0; i < 2; i++)
    {
        for (int row = 0; row < 3; row++)
        {
            if ((gameArea[row][0] == symbols[i])and(gameArea[row][1] == symbols[i])and(gameArea[row][2] == symbols[i]))
            {                
                winner = symbols[i];
                finResultStyle(row, 0, "row");
                return;
            }
        }
        for (int col = 0; col < 3; col++)
        {
            if ((gameArea[0][col] == symbols[i])and(gameArea[1][col] == symbols[i])and(gameArea[2][col] == symbols[i]))
            {
                winner = symbols[i];
                finResultStyle(0, col, "column");
                return;
            }
        }
        if ((gameArea[0][0] == symbols[i])and(gameArea[1][1] == symbols[i])and(gameArea[2][2] == symbols[i]))
        {           
            winner = symbols[i];
            finResultStyle(0, 0, "main diagonal");
            return;
        }
        if ((gameArea[0][2] == symbols[i])and(gameArea[1][1] == symbols[i])and(gameArea[2][0] == symbols[i]))
        {    
            winner = symbols[i];
            finResultStyle(0, 0, "second diagonal");
            return;
        }
    }
    if (progress == 9)
    {
        stop = true;
    }
}

void Widget::finResultStyle(int row, int col, QString line)
{
    QString style;
    stop = true;
    if (winner == player)
    {
        style = (player == 'X') ? StyleHelper::getCrossVictoryStyle() : StyleHelper::getZeroVictoryStyle();
    }
    else
    {
        style = (player == 'X') ? StyleHelper::getZeroLostStyle() : StyleHelper::getCrossLostStyle();
    }
    if (line ==  "row")
        for (int i = 0; i < 3; i++)
            changeButtonStyle(row, i, style);
    else if (line == "column")
        for (int i = 0; i < 3; i++)
            changeButtonStyle(i, col, style);
    else if (line == "main diagonal")
        for (int i = 0; i < 3; i++)
            changeButtonStyle(i, i, style);
    else if (line == "second diagonal")
        for (int i = 0; i < 3; i++)
            changeButtonStyle(i, 2 - i, style);
}

void Widget::endGame()
{

    if (stop)
    {
        if (winner == player)
        {
            ui->messageLabel->setText("Победа!");
            ui->messageLabel->setStyleSheet(StyleHelper::getVictoryMessageStyle());
        }
        else if (winner == '-')
        {
            ui->messageLabel->setText("Ничья!");
        }
        else
        {
            ui->messageLabel->setText("Проиграл!");
            ui->messageLabel->setStyleSheet(StyleHelper::getLostMessageStyle());
        }
        playerLocked = true;
        ui->startButton->setText("Играть");
        ui->startButton->setStyleSheet(StyleHelper::getStartButtonsStyle());
        ui->leftButton->setDisabled(false);
        ui->rightButton->setDisabled(false);
        gameStart = false;
    }
}

void Widget::onComputerSlot()
{
    char comp;
    QString style;
    if (player == 'X')
    {
        comp = '0';
        style = StyleHelper::getZeroNormalStyle();
    }
    else
    {
        comp = 'X';
        style = StyleHelper::getCrossNormalStyle();
    }
    timer->stop();
    while (true)
    {
        int row = rand() % 3;
        int column = rand() % 3;
        if (gameArea[row][column] == '-')
        {
            gameArea[row][column] = comp;
            changeButtonStyle(row, column, style);
            ui->messageLabel->setText("Ваш ход");
            progress++;
            checkGameStop();
            endGame();
            playerLocked = false;
            break;
        }
    }
}

void Widget::on_aboutButton_clicked()
{
    if (checkTab)
    {
        ui->tabWidget->setCurrentIndex(1);
        checkTab = false;
    }
    else
    {
        ui->tabWidget->setCurrentIndex(0);
        checkTab = true;
    }
}

