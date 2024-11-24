#include <QtWidgets>
#include <QDomDocument>

/**
 * @class InputDialog
 * @brief Диалоговое окно для ввода параметров температуры, давления и влажности.
 */
class InputDialog : public QDialog {
public:
    /**
     * @brief Конструктор класса InputDialog.
     * @param parent Указатель на родительский виджет.
     */
    explicit InputDialog(QWidget *parent = nullptr) : QDialog(parent) {
        createUI();
    }

    /**
     * @brief Получает значение температуры.
     * @return Значение температуры.
     */
    int getTemperature() const {
        return temperatureEdit->text().toInt();
    }

    /**
     * @brief Получает значение давления.
     * @return Значение давления.
     */
    int getPressure() const {
        return pressureEdit->text().toInt();
    }

    /**
     * @brief Получает значение влажности.
     * @return Значение влажности.
     */
    int getHumidity() const {
        return humidityEdit->text().toInt();
    }

private:
    /**
     * @brief Создает пользовательский интерфейс.
     */
    void createUI() {
        setWindowTitle("Ввод параметров");
        setMaximumSize(1024, 768);
        setMinimumSize(800, 600);
        resize(800, 600);

        auto *mainLayout = new QVBoxLayout;

        auto *temperatureLayout = new QFormLayout;
        auto *pressureLayout = new QFormLayout;
        auto *humidityLayout = new QFormLayout;
        temperatureEdit = new QLineEdit;
        pressureEdit = new QLineEdit;
        humidityEdit = new QLineEdit;

        auto font = QFont("Arial", 20);
        temperatureEdit->setFont(font);
        pressureEdit->setFont(font);
        humidityEdit->setFont(font);
        auto *text = new QLabel("Температура(от 16 до 30):");
        text->setFont(font);
        temperatureLayout->addRow(text);
        text = new QLabel("°C");
        text->setFont(font);
        temperatureLayout->addRow(temperatureEdit, text);
        text = new QLabel("Давление(от 0):");
        text->setFont(font);
        pressureLayout->addRow(text);
        text = new QLabel("Па");
        text->setFont(font);
        pressureLayout->addRow(pressureEdit, text);
        text = new QLabel("Влажность(от 0 до 100):");
        text->setFont(font);
        humidityLayout->addRow(text);
        text = new QLabel("%");
        text->setFont(font);
        humidityLayout->addRow(humidityEdit, text);

        mainLayout->addLayout(temperatureLayout);
        mainLayout->addLayout(pressureLayout);
        mainLayout->addLayout(humidityLayout);

        auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        buttonBox->setFont(font);
        connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

        mainLayout->addWidget(buttonBox);
        setLayout(mainLayout);
    }

    QLineEdit *temperatureEdit; /**< Поле ввода для температуры. */
    QLineEdit *pressureEdit; /**< Поле ввода для давления. */
    QLineEdit *humidityEdit; /**< Поле ввода для влажности. */
};

/**
 * @class AirConditioningControl
 * @brief Виджет для управления кондиционированием воздуха.
 */
class AirConditioningControl : public QWidget {
public:
    /**
     * @brief Конструктор класса AirConditioningControl.
     * @param initialTemperature Начальное значение температуры.
     * @param initialPressure Начальное значение давления.
     * @param initialHumidity Начальное значение влажности.
     * @param parent Указатель на родительский виджет.
     */
    explicit AirConditioningControl(int initialTemperature, int initialPressure, int initialHumidity,
                                    QWidget *parent = nullptr)
        : QWidget(parent), temperature(initialTemperature), pressure(initialPressure), humidity(initialHumidity),
          temperatureScene(new QGraphicsScene(this)), humidityScene(new QGraphicsScene(this)),
          coordsScene(new QGraphicsScene(this)) {
        createUI();
        loadSettingsFromXml();
    }

protected:
    /**
     * @brief Обработчик события закрытия окна.
     * @param event Событие закрытия.
     */
    void closeEvent(QCloseEvent *event) override {
        saveSettingsToXml();
        event->accept();
    }

private slots:
    /**
     * @brief Обновляет отображение температуры.
     * @param value Новое значение температуры.
     */
    void updateTemperature(int value) {
        double tempCelsius = value;
        double tempKelvin = tempCelsius + 273.15;
        double tempFahrenheit = (tempCelsius * 9 / 5) + 32;

        QString tempText;
        switch (temperatureUnitCombo->currentIndex()) {
            case 0:
                tempText = QString("Температура: %1°C").arg(tempCelsius);
                break;
            case 1:
                tempText = QString("Температура: %1 K").arg(tempKelvin);
                break;
            case 2:
                tempText = QString("Температура: %1°F").arg(tempFahrenheit);
                break;
        }
        temperatureTextItem->setPlainText(tempText);

        double minTemp = temperatureSlider->minimum();
        double maxTemp = temperatureSlider->maximum();
        double fillHeight = (tempCelsius - minTemp) / (maxTemp - minTemp) * temperatureRect->rect().height();
        temperatureFillRect->setRect(temperatureRect->rect().x(),
                                     temperatureRect->rect().y() + temperatureRect->rect().height() - fillHeight,
                                     temperatureRect->rect().width(), fillHeight);
    }

    /**
     * @brief Обновляет единицы измерения температуры.
     */
    void updateTemperatureUnits() {
        updateTemperature(temperatureSlider->value());
    }

    /**
     * @brief Обновляет единицы измерения давления.
     */
    void updatePressureUnits() {
        double pressurePa = pressure;
        double pressureMmHg = pressurePa * 0.00750062;

        QString pressureText;
        switch (pressureUnitCombo->currentIndex()) {
            case 0:
                pressureText = QString("%1 Па").arg(pressurePa);
                break;
            case 1:
                pressureText = QString("%1 мм рт. ст.").arg(pressureMmHg);
                break;
        }
        pressureLabel->setText(pressureText);
    }

    /**
     * @brief Переключает состояние питания.
     */
    void togglePower() {
        powerButton->setText(powerButton->text() == "Включить" ? "Выключить" : "Включить");
    }

    /**
     * @brief Переключает тему оформления.
     */
    void toggleTheme() {
        if (themeButton->text() == "Светлая тема") {
            themeButton->setText("Темная тема");
            applyLightTheme();
        } else {
            themeButton->setText("Светлая тема");
            applyDarkTheme();
        }
    }

    /**
     * @brief Перемещает точку вверх.
     */
    void movePointUp() {
        if (point->y() > -150)
            point->moveBy(0, -10);
    }

    /**
     * @brief Перемещает точку вниз.
     */
    void movePointDown() {
        if (point->y() < 150)
            point->moveBy(0, 10);
    }

    /**
     * @brief Перемещает точку влево.
     */
    void movePointLeft() {
        if (point->x() > -150)
            point->moveBy(-10, 0);
    }

    /**
     * @brief Перемещает точку вправо.
     */
    void movePointRight() {
        if (point->x() < 150)
            point->moveBy(10, 0);
    }

private:
    /**
     * @brief Создает пользовательский интерфейс.
     */
    void createUI() {
        setWindowTitle("Управление кондиционированием");
        setMaximumSize(1024, 768);
        setMinimumSize(800, 600);
        resize(1024, 768);

        auto *mainLayout = new QVBoxLayout;
        font = QFont("Arial", 16);
        setFont(font);

        auto *pressureLayout = new QHBoxLayout;
        auto *pressureLabelText = new QLabel("Давление:");
        pressureLabel = new QLabel(QString("%1 Па").arg(pressure));
        pressureUnitCombo = new QComboBox;
        pressureUnitCombo->addItem("Па");
        pressureUnitCombo->addItem("мм рт. ст.");
        pressureLayout->addWidget(pressureLabelText);
        pressureLayout->addWidget(pressureLabel);
        pressureLayout->addWidget(pressureUnitCombo);
        mainLayout->addLayout(pressureLayout);

        auto *contentLayout = new QHBoxLayout;

        auto *leftSideLayout = new QVBoxLayout;
        auto *temperatureLayout = new QHBoxLayout;
        auto *temperatureLabelText = new QLabel("Температура:");
        temperatureSlider = new QSlider(Qt::Horizontal);
        temperatureSlider->setRange(16, 30);
        temperatureSlider->setValue(temperature);
        temperatureUnitCombo = new QComboBox;
        temperatureUnitCombo->addItem("°C");
        temperatureUnitCombo->addItem("K");
        temperatureUnitCombo->addItem("°F");
        temperatureLayout->addWidget(temperatureLabelText);
        temperatureLayout->addWidget(temperatureSlider);
        temperatureLayout->addWidget(temperatureUnitCombo);
        leftSideLayout->addLayout(temperatureLayout);
        contentLayout->addLayout(leftSideLayout);

        auto *rightSideLayout = new QVBoxLayout;
        auto *airflowLabelText = new QLabel("Направление обдува:");
        auto *airflowButtonsLayout = new QHBoxLayout;
        upButton = new QPushButton("Вверх");
        downButton = new QPushButton("Вниз");
        leftButton = new QPushButton("Влево");
        rightButton = new QPushButton("Вправо");
        airflowButtonsLayout->addWidget(upButton);
        airflowButtonsLayout->addWidget(downButton);
        airflowButtonsLayout->addWidget(leftButton);
        airflowButtonsLayout->addWidget(rightButton);
        rightSideLayout->addWidget(airflowLabelText);
        rightSideLayout->addLayout(airflowButtonsLayout);
        contentLayout->addLayout(rightSideLayout);

        mainLayout->addLayout(contentLayout);

        auto *buttonsLayout = new QHBoxLayout;
        powerButton = new QPushButton("Включить");
        themeButton = new QPushButton("Темная тема");
        buttonsLayout->addWidget(powerButton);
        buttonsLayout->addWidget(themeButton);
        mainLayout->addLayout(buttonsLayout);

        auto *viewsLayout = new QHBoxLayout;
        auto *viewsLayout2 = new QVBoxLayout;
        temperatureView = new QGraphicsView(temperatureScene);
        auto *humidityView = new QGraphicsView(humidityScene);
        auto *coordsView = new QGraphicsView(coordsScene);
        viewsLayout2->addWidget(temperatureView);
        viewsLayout2->addWidget(humidityView);
        viewsLayout->addLayout(viewsLayout2);
        viewsLayout->addWidget(coordsView);

        temperatureRect = new QGraphicsRectItem(0, 0, 300, 100);
        temperatureScene->addItem(temperatureRect);

        temperatureFillRect = new QGraphicsRectItem(temperatureRect);
        temperatureFillRect->setBrush(QBrush(Qt::green));

        temperatureTextItem = new QGraphicsTextItem(temperatureRect);
        temperatureTextItem->setFont(font);

        auto *humidityRect = new QGraphicsRectItem(0, 0, 300, 100);
        humidityScene->addItem(humidityRect);

        auto *humidityFillRect = new QGraphicsRectItem(humidityRect);
        humidityFillRect->setBrush(QBrush(Qt::blue));
        double fillHeight = humidity / 100.0 * humidityRect->rect().height();
        humidityFillRect->setRect(humidityRect->rect().x(),
                                  humidityRect->rect().y() + humidityRect->rect().height() - fillHeight,
                                  humidityRect->rect().width(), fillHeight);

        auto *humidityTextItem = new QGraphicsTextItem(QString("Влажность: %1%").arg(humidity), humidityRect);
        humidityTextItem->setFont(font);

        auto *xAxis = new QGraphicsLineItem(0, 150, 300, 150);
        auto *yAxis = new QGraphicsLineItem(150, 0, 150, 300);
        point = new QGraphicsEllipseItem(145, 145, 10, 10);
        point->setBrush(QBrush(Qt::red));
        coordsScene->addItem(xAxis);
        coordsScene->addItem(yAxis);
        coordsScene->addItem(point);

        auto *xLabel = new QGraphicsTextItem("X");
        xLabel->setPos(300, 150);
        coordsScene->addItem(xLabel);

        auto *yLabel = new QGraphicsTextItem("Y");
        yLabel->setPos(150, 0);
        coordsScene->addItem(yLabel);

        mainLayout->addLayout(viewsLayout);
        setLayout(mainLayout);

        connect(temperatureSlider, &QSlider::valueChanged, this, &AirConditioningControl::updateTemperature);
        connect(temperatureUnitCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
                &AirConditioningControl::updateTemperatureUnits);
        connect(pressureUnitCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
                &AirConditioningControl::updatePressureUnits);
        connect(powerButton, &QPushButton::clicked, this, &AirConditioningControl::togglePower);
        connect(themeButton, &QPushButton::clicked, this, &AirConditioningControl::toggleTheme);
        connect(upButton, &QPushButton::clicked, this, &AirConditioningControl::movePointUp);
        connect(downButton, &QPushButton::clicked, this, &AirConditioningControl::movePointDown);
        connect(leftButton, &QPushButton::clicked, this, &AirConditioningControl::movePointLeft);
        connect(rightButton, &QPushButton::clicked, this, &AirConditioningControl::movePointRight);

        updateTemperature(temperature);
    }

    /**
     * @brief Применяет светлую тему оформления.
     */
    void applyLightTheme() {
        QPalette palette;
        palette.setColor(QPalette::Window, QColor(255, 255, 255));
        palette.setColor(QPalette::WindowText, QColor(0, 0, 0));
        palette.setColor(QPalette::Base, QColor(240, 240, 240));
        palette.setColor(QPalette::AlternateBase, QColor(255, 255, 255));
        palette.setColor(QPalette::ToolTipBase, QColor(255, 255, 255));
        palette.setColor(QPalette::ToolTipText, QColor(0, 0, 0));
        palette.setColor(QPalette::Text, QColor(0, 0, 0));
        palette.setColor(QPalette::Button, QColor(240, 240, 240));
        palette.setColor(QPalette::ButtonText, QColor(0, 0, 0));
        palette.setColor(QPalette::BrightText, QColor(255, 0, 0));
        palette.setColor(QPalette::Link, QColor(0, 0, 255));
        palette.setColor(QPalette::Highlight, QColor(0, 120, 215));
        palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
        setPalette(palette);
        updateSceneColors(Qt::black);
    }

    /**
     * @brief Применяет темную тему оформления.
     */
    void applyDarkTheme() {
        QPalette palette;
        palette.setColor(QPalette::Window, QColor(53, 53, 53));
        palette.setColor(QPalette::WindowText, QColor(255, 255, 255));
        palette.setColor(QPalette::Base, QColor(25, 25, 25));
        palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        palette.setColor(QPalette::ToolTipBase, QColor(255, 255, 255));
        palette.setColor(QPalette::ToolTipText, QColor(255, 255, 255));
        palette.setColor(QPalette::Text, QColor(255, 255, 255));
        palette.setColor(QPalette::Button, QColor(53, 53, 53));
        palette.setColor(QPalette::ButtonText, QColor(255, 255, 255));
        palette.setColor(QPalette::BrightText, QColor(255, 0, 0));
        palette.setColor(QPalette::Link, QColor(42, 130, 218));
        palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        palette.setColor(QPalette::HighlightedText, QColor(0, 0, 0));
        setPalette(palette);
        updateSceneColors(Qt::white);
    }

    /**
     * @brief Обновляет цвета сцены.
     * @param color Новый цвет.
     */
    void updateSceneColors(const QColor &color) {
        for (auto obj: temperatureScene->items() + humidityScene->items() + coordsScene->items()) {
            switch (obj->type()) {
                case QGraphicsRectItem::Type:
                    qgraphicsitem_cast<QGraphicsRectItem *>(obj)->setPen(QPen(color));
                    break;
                case QGraphicsLineItem::Type:
                    qgraphicsitem_cast<QGraphicsLineItem *>(obj)->setPen(QPen(color));
                    break;
                case QGraphicsTextItem::Type:
                    qgraphicsitem_cast<QGraphicsTextItem *>(obj)->setDefaultTextColor(color);
                    break;
                default:
                    break;
            }
        }
    }

    /**
     * @brief Сохраняет настройки в XML файл.
     */
    void saveSettingsToXml() {
        QDomDocument doc;
        QDomElement root = doc.createElement("Settings");
        doc.appendChild(root);

        QDomElement temperatureUnitElement = doc.createElement("TemperatureUnit");
        temperatureUnitElement.setAttribute("index", temperatureUnitCombo->currentIndex());
        root.appendChild(temperatureUnitElement);

        QDomElement pressureUnitElement = doc.createElement("PressureUnit");
        pressureUnitElement.setAttribute("index", pressureUnitCombo->currentIndex());
        root.appendChild(pressureUnitElement);

        QFile file("settings.xml");
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QTextStream stream(&file);
            stream << doc.toString();
            file.close();
        }
    }

    /**
     * @brief Загружает настройки из XML файла.
     */
    void loadSettingsFromXml() {
        QFile file("settings.xml");
        if (file.open(QIODevice::ReadOnly)) {
            QDomDocument doc;
            if (doc.setContent(&file)) {
                QDomElement root = doc.documentElement();

                QDomElement temperatureUnitElement = root.firstChildElement("TemperatureUnit");
                if (!temperatureUnitElement.isNull()) {
                    int index = temperatureUnitElement.attribute("index").toInt();
                    temperatureUnitCombo->setCurrentIndex(index);
                }

                QDomElement pressureUnitElement = root.firstChildElement("PressureUnit");
                if (!pressureUnitElement.isNull()) {
                    int index = pressureUnitElement.attribute("index").toInt();
                    pressureUnitCombo->setCurrentIndex(index);
                }
            }
            file.close();
        }
    }

    QGraphicsScene *temperatureScene; /**< Сцена для отображения температуры. */
    QGraphicsScene *humidityScene; /**< Сцена для отображения влажности. */
    QGraphicsScene *coordsScene; /**< Сцена для отображения направления обдува. */
    QGraphicsView *temperatureView; /**< Виджет для отображения temperatureScene. */
    QSlider *temperatureSlider; /**< Ползунок для управления температурой. */
    QPushButton *upButton; /**< Кнопка для перемещения точки вверх. */
    QPushButton *downButton; /**< Кнопка для перемещения точки вниз. */
    QPushButton *leftButton; /**< Кнопка для перемещения точки влево. */
    QPushButton *rightButton; /**< Кнопка для перемещения точки вправо. */
    QPushButton *powerButton; /**< Кнопка для управления питанием. */
    QPushButton *themeButton; /**< Кнопка для переключения темы. */
    QComboBox *temperatureUnitCombo; /**< Выпадающий список для выбора единиц температуры. */
    QComboBox *pressureUnitCombo; /**< Выпадающий список для выбора единиц давления. */
    QGraphicsTextItem *temperatureTextItem; /**< Текстовый элемент для отображения температуры. */
    QLabel *pressureLabel; /**< Лейбл для отображения давления. */
    QGraphicsRectItem *temperatureRect; /**< Прямоугольник для отображения температуры. */
    QGraphicsRectItem *temperatureFillRect; /**< Заполняемый прямоугольник для отображения температуры. */
    QGraphicsEllipseItem *point; /**< Точка для отображения направления обдува. */
    QFont font; /**< Основная тема текста. */

    int temperature; /**< Температура. */
    int pressure; /**< Давление. */
    int humidity; /**< Влажность. */
};

/**
 * @brief Главная функция программы.
 * @param argc Количество аргументов командной строки.
 * @param argv Аргументы командной строки.
 * @return Код возврата.
 */
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    InputDialog inputDialog = InputDialog();
    if (inputDialog.exec() == QDialog::Accepted) {
        int initialTemperature = std::max(std::min(inputDialog.getTemperature(), 30), 16);
        int initialPressure = std::max(inputDialog.getPressure(), 0);
        int initialHumidity = std::max(std::min(inputDialog.getHumidity(), 100), 0);

        AirConditioningControl window(initialTemperature, initialPressure, initialHumidity);
        window.show();

        return app.exec();
    }
    return 0;
}
