#include "../include/GameWindow.h"
#include "../include/DSA.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QKeyEvent>
#include <QInputDialog>
#include <QHeaderView>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QLineEdit>
#include <cstring>
#include <QUrl>
using namespace std;

GameWindow::GameWindow(QWidget* parent) : QMainWindow(parent), scoreCount(0) {
    currentPlayerName = "Anonymous";
    setWindowTitle("Fireboy & Watergirl – Forest Temple");
    resize(1000, 760);
    eng = new GameEngine(this);
    connect(eng, &GameEngine::frameReady,    this, &GameWindow::onFrameReady);
    connect(eng, &GameEngine::stateChanged,  this, &GameWindow::onStateChanged);

    playlist = new QMediaPlaylist(this);
    playlist->addMedia(QUrl("qrc:/sounds/bgm.mp3"));
    playlist->setPlaybackMode(QMediaPlaylist::Loop);

    bgMusic = new QMediaPlayer(this);
    bgMusic->setPlaylist(playlist);
    bgMusic->setVolume(50);
    bgMusic->play();

    buildUI();
    loadScores();
}

void GameWindow::buildUI() {
    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    // ── Page 0: Game ─────────────────────────────────────────
    QWidget* gamePage = new QWidget;
    QVBoxLayout* gl = new QVBoxLayout(gamePage);
    gl->setContentsMargins(0,0,0,0);
    renderer = new GameRenderer(eng, gamePage);
    gl->addWidget(renderer);
    stack->addWidget(gamePage);

    // ── Page 1: Main Menu ────────────────────────────────────
    QWidget* menuPage = new QWidget;
    menuPage->setObjectName("menuPage");
    QVBoxLayout* ml = new QVBoxLayout(menuPage);
    ml->setAlignment(Qt::AlignCenter); ml->setSpacing(16);

    QLabel* title = new QLabel(" Fireboy  &  Watergirl");
    title->setObjectName("titleLabel"); title->setAlignment(Qt::AlignCenter);
    ml->addWidget(title);

    QLabel* sub = new QLabel("Forest Temple Edition");
    sub->setObjectName("subLabel"); sub->setAlignment(Qt::AlignCenter);
    ml->addWidget(sub);
    ml->addSpacing(24);

    auto makeBtn = [&](const QString& txt, const char* id) {
        QPushButton* b = new QPushButton(txt);
        b->setObjectName(id); b->setFixedSize(240, 50);
        ml->addWidget(b, 0, Qt::AlignCenter); return b;
    };
    auto* btnPlay = makeBtn("  Play Game",      "btnPlay");
    auto* btnLB   = makeBtn("  Leaderboard",    "btnSecondary");
    auto* btnQuit = makeBtn("  Quit",           "btnQuit");

    connect(btnPlay, &QPushButton::clicked, this, &GameWindow::startGame);
    connect(btnLB,   &QPushButton::clicked, this, &GameWindow::showLeaderboard);
    connect(btnQuit, &QPushButton::clicked, qApp, &QApplication::quit);

    ml->addSpacing(20);
    QLabel* ctrl = new QLabel(
        "<b>Controls</b><br>"
        " Fireboy : ← ↑ →<br>"
        " Watergirl : A W D<br>"
        "Hint path : H &nbsp;|&nbsp; Pause : Esc");
    ctrl->setObjectName("ctrlLabel"); ctrl->setAlignment(Qt::AlignCenter);
    ml->addWidget(ctrl);
    stack->addWidget(menuPage);

    // ── Page 2: Leaderboard ──────────────────────────────────
    QWidget* lbPage = new QWidget;
    lbPage->setObjectName("lbPage");
    QVBoxLayout* ll = new QVBoxLayout(lbPage);
    ll->setContentsMargins(40,30,40,30); ll->setSpacing(14);

    QLabel* lbTitle = new QLabel("  Leaderboard");
    lbTitle->setObjectName("titleLabel"); lbTitle->setAlignment(Qt::AlignCenter);
    ll->addWidget(lbTitle);

    lbTable = new QTableWidget(0, 4, lbPage);
    lbTable->setObjectName("lbTable");
    lbTable->setHorizontalHeaderLabels({"Rank","Name","Score","Level"});
    lbTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    lbTable->verticalHeader()->hide();
    lbTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    lbTable->setAlternatingRowColors(true);
    ll->addWidget(lbTable);

    QPushButton* back = new QPushButton("◀  Back to Menu");
    back->setObjectName("btnSecondary"); back->setFixedSize(200, 44);
    ll->addWidget(back, 0, Qt::AlignCenter);
    connect(back, &QPushButton::clicked, this, &GameWindow::showMenu);
    stack->addWidget(lbPage);

    // ── Page 3: Name Entry ───────────────────────────────────
    QWidget* namePage = new QWidget;
    namePage->setObjectName("namePage");
    QVBoxLayout* nl = new QVBoxLayout(namePage);
    nl->setAlignment(Qt::AlignCenter); nl->setSpacing(20);

    QLabel* nameTitle = new QLabel("Welcome to Fireboy & Watergirl!");
    nameTitle->setObjectName("titleLabel"); nameTitle->setAlignment(Qt::AlignCenter);
    nl->addWidget(nameTitle);

    QLabel* nameSub = new QLabel("Please enter your player name:");
    nameSub->setObjectName("subLabel"); nameSub->setAlignment(Qt::AlignCenter);
    nl->addWidget(nameSub);

    nameInput = new QLineEdit();
    nameInput->setObjectName("nameInput");
    nameInput->setPlaceholderText("Enter Name...");
    nameInput->setFixedSize(300, 50);
    nameInput->setAlignment(Qt::AlignCenter);
    nl->addWidget(nameInput, 0, Qt::AlignCenter);

    QPushButton* btnSubmit = new QPushButton("Continue");
    btnSubmit->setObjectName("btnSubmit");
    btnSubmit->setFixedSize(200, 50);
    nl->addWidget(btnSubmit, 0, Qt::AlignCenter);
    connect(btnSubmit, &QPushButton::clicked, this, &GameWindow::submitName);
    connect(nameInput, &QLineEdit::returnPressed, this, &GameWindow::submitName);

    stack->addWidget(namePage);

    stack->setCurrentIndex(3); // start on name entry screen
}

void GameWindow::startGame() {
    stack->setCurrentIndex(0);
    renderer->setFocus();
    eng->start();
}
void GameWindow::showMenu()        { stack->setCurrentIndex(1); }
void GameWindow::showLeaderboard() { refreshLeaderboard(); stack->setCurrentIndex(2); }

void GameWindow::submitName() {
    QString name = nameInput->text().trimmed();
    if (!name.isEmpty()) {
        currentPlayerName = name;
    }
    stack->setCurrentIndex(1); // Go to main menu
}

void GameWindow::onFrameReady()    { renderer->update(); }

void GameWindow::onStateChanged(int s) {
    renderer->update();
    if (s == STATE_WIN || s == STATE_GAMEOVER) {
        saveScore(currentPlayerName, eng->score,
                  eng->levels.current ? eng->levels.current->data.num : 1,
                  eng->elapsed);
    }
}

void GameWindow::saveScore(const QString& name, int score, int level, float time) {
    if (scoreCount < MAX_SCORES) {
        ScoreEntry& e = scores[scoreCount++];
        strncpy(e.name, name.toUtf8().constData(), 31); e.name[31]=0;
        e.score = score; e.level = level; e.timeSec = time;
    }
    // DSA: use QuickSort for larger lists, BubbleSort for small
    if (scoreCount > 5) quickSort(scores, 0, scoreCount-1);
    else                bubbleSort(scores, scoreCount);

    // Save to file
    QFile f("scores.txt");
    if (f.open(QIODevice::WriteOnly|QIODevice::Text)) {
        QTextStream out(&f);
        for (int i = 0; i < scoreCount; i++)
            out << scores[i].name << "," << scores[i].score
                << "," << scores[i].level << "," << scores[i].timeSec << "\n";
    }
}

void GameWindow::loadScores() {
    scoreCount = 0;
    QFile f("scores.txt");
    if (!f.open(QIODevice::ReadOnly|QIODevice::Text)) return;
    QTextStream in(&f);
    while (!in.atEnd() && scoreCount < MAX_SCORES) {
        QStringList p = in.readLine().split(",");
        if (p.size() < 4) continue;
        strncpy(scores[scoreCount].name, p[0].toUtf8().constData(), 31);
        scores[scoreCount].score   = p[1].toInt();
        scores[scoreCount].level   = p[2].toInt();
        scores[scoreCount].timeSec = p[3].toFloat();
        scoreCount++;
    }
    if (scoreCount > 0) quickSort(scores, 0, scoreCount-1);
}

void GameWindow::refreshLeaderboard() {
    lbTable->setRowCount(scoreCount);
    for (int i = 0; i < scoreCount; i++) {
        // DSA: BinarySearch to find rank position
        int rank = binarySearch(scores, scoreCount, scores[i].score) + 1;
        lbTable->setItem(i,0,new QTableWidgetItem(QString::number(rank)));
        lbTable->setItem(i,1,new QTableWidgetItem(scores[i].name));
        lbTable->setItem(i,2,new QTableWidgetItem(QString::number(scores[i].score)));
        lbTable->setItem(i,3,new QTableWidgetItem(QString::number(scores[i].level)));
        for (int c=0;c<4;c++) if (auto* it=lbTable->item(i,c)) it->setTextAlignment(Qt::AlignCenter);
    }
}

void GameWindow::keyPressEvent(QKeyEvent* e) {
    int s = eng->state;
    if (e->key()==Qt::Key_Return || e->key()==Qt::Key_Space) {
        if (s==STATE_MENU)   { startGame(); return; }
        if (s==STATE_PAUSED) { eng->resume(); return; }
        if (s==STATE_WIN && eng->levels.current && eng->levels.current->next)
            { eng->nextLevel(); return; }
    }
    if (e->key()==Qt::Key_R && (s==STATE_DEAD||s==STATE_GAMEOVER||s==STATE_WIN))
        { eng->start(); return; }
    if (e->key()==Qt::Key_Escape && s==STATE_PAUSED) { eng->resume(); return; }
    eng->keyPress(e->key());
}
void GameWindow::keyReleaseEvent(QKeyEvent* e) { eng->keyRelease(e->key()); }
void GameWindow::closeEvent(QCloseEvent* e)    { e->accept(); }
