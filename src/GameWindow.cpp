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
#include <QTimer>
using namespace std;

// ── GameWindow Constructor ─────────────────────────────────────
// Sets up the main window, the game engine, and the media players.
GameWindow::GameWindow(QWidget* parent) : QMainWindow(parent), scoreCount(0)
{
    currentPlayerName = "Anonymous";
    setWindowTitle("Fireboy & Watergirl :)");
    resize(1000, 760);
    
    // Create the game engine and connect its signals to our functions
    eng = new GameEngine(this);
    connect(eng, &GameEngine::frameReady,   this, &GameWindow::onFrameReady);
    connect(eng, &GameEngine::stateChanged, this, &GameWindow::onStateChanged);

    // Setup background music based on Qt version
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    playlist = new QMediaPlaylist(this);
    playlist->addMedia(QUrl("qrc:/sounds/bgm.mp3"));
    playlist->setPlaybackMode(QMediaPlaylist::Loop);
    bgMusic = new QMediaPlayer(this);
    bgMusic->setPlaylist(playlist);
    bgMusic->setVolume(50);
    QTimer::singleShot(0, this, [this]() { if (bgMusic) bgMusic->play(); });
#else
    bgMusic = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    bgMusic->setAudioOutput(audioOutput);
    bgMusic->setSource(QUrl("qrc:/sounds/bgm.mp3"));
    audioOutput->setVolume(0.5);
    bgMusic->setLoops(-1);
    QTimer::singleShot(0, this, [this]() { if (bgMusic) bgMusic->play(); });
#endif

    // Build the user interface and load previous scores
    buildUI();
    loadScores();
}

// ── buildUI ───────────────────────────────────────────────────
// Builds the 4 main screens (Game, Menu, Leaderboard, Name Entry)
// using a QStackedWidget to easily switch between them.
void GameWindow::buildUI()
{
    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    // ── Page 0: Game Screen ──
    QWidget* gamePage = new QWidget;
    QVBoxLayout* gl = new QVBoxLayout(gamePage);
    gl->setContentsMargins(0,0,0,0);
    renderer = new GameRenderer(eng, gamePage); // The widget that draws the game
    gl->addWidget(renderer);
    stack->addWidget(gamePage);

    // ── Page 1: Main Menu ──
    QWidget* menuPage = new QWidget;
    menuPage->setObjectName("menuPage");
    QVBoxLayout* ml = new QVBoxLayout(menuPage);
    ml->setAlignment(Qt::AlignCenter); 
    ml->setSpacing(16);

    QLabel* title = new QLabel(" Fireboy  &  Watergirl");
    title->setObjectName("titleLabel"); 
    title->setAlignment(Qt::AlignCenter);
    ml->addWidget(title);

    QLabel* sub = new QLabel("Forest Temple Edition");
    sub->setObjectName("subLabel"); 
    sub->setAlignment(Qt::AlignCenter);
    ml->addWidget(sub);
    ml->addSpacing(24);

    // Helper to make menu buttons easily
    auto makeBtn = [&](const QString& txt, const char* id) {
        QPushButton* b = new QPushButton(txt);
        b->setObjectName(id); 
        b->setFixedSize(240, 50);
        ml->addWidget(b, 0, Qt::AlignCenter); 
        return b;
    };
    
    QPushButton* btnPlay = makeBtn("  Play Game",   "btnPlay");
    QPushButton* btnLB   = makeBtn("  Leaderboard", "btnSecondary");
    QPushButton* btnQuit = makeBtn("  Quit",        "btnQuit");

    connect(btnPlay, &QPushButton::clicked, this, &GameWindow::startGame);
    connect(btnLB,   &QPushButton::clicked, this, &GameWindow::showLeaderboard);
    connect(btnQuit, &QPushButton::clicked, qApp, &QApplication::quit);

    ml->addSpacing(20);
    QLabel* ctrl = new QLabel(
        "<b>Controls</b><br>"
        " Fireboy : ← ↑ →<br>"
        " Watergirl : A W D<br>"
        "Hint path : H &nbsp;|&nbsp; Pause : Esc");
    ctrl->setObjectName("ctrlLabel"); 
    ctrl->setAlignment(Qt::AlignCenter);
    ml->addWidget(ctrl);
    stack->addWidget(menuPage);

    // ── Page 2: Leaderboard ──
    QWidget* lbPage = new QWidget;
    lbPage->setObjectName("lbPage");
    QVBoxLayout* ll = new QVBoxLayout(lbPage);
    ll->setContentsMargins(40,30,40,30); 
    ll->setSpacing(14);

    QLabel* lbTitle = new QLabel("  Leaderboard");
    lbTitle->setObjectName("titleLabel"); 
    lbTitle->setAlignment(Qt::AlignCenter);
    ll->addWidget(lbTitle);

    lbTable = new QTableWidget(0, 4, lbPage);
    lbTable->setObjectName("lbTable");
    lbTable->setHorizontalHeaderLabels({"Rank", "Name", "Score", "Level"});
    lbTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    lbTable->verticalHeader()->hide();
    lbTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    lbTable->setAlternatingRowColors(true);
    ll->addWidget(lbTable);

    QPushButton* back = new QPushButton("◀  Back to Menu");
    back->setObjectName("btnSecondary"); 
    back->setFixedSize(200, 44);
    ll->addWidget(back, 0, Qt::AlignCenter);
    connect(back, &QPushButton::clicked, this, &GameWindow::showMenu);
    stack->addWidget(lbPage);

    // ── Page 3: Name Entry ──
    QWidget* namePage = new QWidget;
    namePage->setObjectName("namePage");
    QVBoxLayout* nl = new QVBoxLayout(namePage);
    nl->setAlignment(Qt::AlignCenter); 
    nl->setSpacing(20);

    QLabel* nameTitle = new QLabel("Welcome to Fireboy & Watergirl!");
    nameTitle->setObjectName("titleLabel"); 
    nameTitle->setAlignment(Qt::AlignCenter);
    nl->addWidget(nameTitle);

    QLabel* nameSub = new QLabel("Please enter your player name:");
    nameSub->setObjectName("subLabel"); 
    nameSub->setAlignment(Qt::AlignCenter);
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

    QPushButton* btnNameLB = new QPushButton("Leaderboard");
    btnNameLB->setObjectName("btnSecondary");
    btnNameLB->setFixedSize(200, 50);
    nl->addWidget(btnNameLB, 0, Qt::AlignCenter);

    QPushButton* btnNameQuit = new QPushButton("Quit");
    btnNameQuit->setObjectName("btnQuit");
    btnNameQuit->setFixedSize(200, 50);
    nl->addWidget(btnNameQuit, 0, Qt::AlignCenter);

    connect(btnSubmit, &QPushButton::clicked, this, &GameWindow::submitName);
    connect(nameInput, &QLineEdit::returnPressed, this, &GameWindow::submitName);
    connect(btnNameLB, &QPushButton::clicked, this, &GameWindow::showLeaderboard);
    connect(btnNameQuit, &QPushButton::clicked, qApp, &QApplication::quit);

    stack->addWidget(namePage);

    // Start by showing the name entry screen
    stack->setCurrentIndex(3); 
}

// ── Screen Transitions ────────────────────────────────────────
void GameWindow::startGame() 
{ 
    eng->levels.current = eng->levels.head;
    stack->setCurrentIndex(0); 
    renderer->setFocus(); // ensure key presses go to the game
    eng->start(); 
}

void GameWindow::showMenu() 
{ 
    stack->setCurrentIndex(1); 
}

void GameWindow::showLeaderboard() 
{ 
    refreshLeaderboard(); 
    stack->setCurrentIndex(2); 
}

void GameWindow::submitName() 
{
    QString name = nameInput->text().trimmed();
    if (!name.isEmpty()) {
        currentPlayerName = name;
    }
    stack->setCurrentIndex(1); // Go to main menu
}

// ── Game Engine Callbacks ─────────────────────────────────────
void GameWindow::onFrameReady() 
{ 
    renderer->update(); // tell the renderer to redraw the screen
}

void GameWindow::onStateChanged(int s) 
{
    renderer->update();
    // Save score only on game over OR on final level win (no next level)
    bool isFinalWin = (s == STATE_WIN) &&
                      !(eng->levels.current && eng->levels.current->next);
    if (isFinalWin || s == STATE_GAMEOVER) {
        int levelNum = eng->levels.current ? eng->levels.current->data.num : 1;
        saveScore(currentPlayerName, eng->score, levelNum);
    }
}

// ── Leaderboard Logic ─────────────────────────────────────────
void GameWindow::saveScore(const QString& name, int score, int level) 
{
    // Add new score if we have room
    if (scoreCount < MAX_SCORES) {
        ScoreEntry& e = scores[scoreCount++];
        strncpy(e.name, name.toUtf8().constData(), 31); 
        e.name[31] = 0;
        e.score   = score; 
        e.level   = level; 
    } else {
        if (score > scores[MAX_SCORES - 1].score) {
            ScoreEntry& e = scores[MAX_SCORES - 1];
            strncpy(e.name, name.toUtf8().constData(), 31); 
            e.name[31] = 0;
            e.score   = score; 
            e.level   = level; 
        }
    }
    
    // Sort all scores highest to lowest using QuickSort
    quickSort(scores, 0, scoreCount - 1);

    // Save them to a text file
    QFile f("scores.txt");
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&f);
        for (int i = 0; i < scoreCount; i++) {
            out << scores[i].name << "," << scores[i].score << "," 
                << scores[i].level << "\n";
        }
    }
}

void GameWindow::loadScores() 
{
    scoreCount = 0;
    QFile f("scores.txt");
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    
    QTextStream in(&f);
    while (!in.atEnd() && scoreCount < MAX_SCORES) {
        QStringList p = in.readLine().split(",");
        if (p.size() < 3) continue;
        strncpy(scores[scoreCount].name, p[0].toUtf8().constData(), 31);
        scores[scoreCount].score   = p[1].toInt();
        scores[scoreCount].level   = p[2].toInt();
        scoreCount++;
    }
    if (scoreCount > 0) quickSort(scores, 0, scoreCount - 1);
}

void GameWindow::refreshLeaderboard() 
{
    lbTable->setRowCount(scoreCount);
    for (int i = 0; i < scoreCount; i++) {
        // Find the rank using Binary Search
        int rank = binarySearch(scores, scoreCount, scores[i].score) + 1;
        
        lbTable->setItem(i, 0, new QTableWidgetItem(QString::number(rank)));
        lbTable->setItem(i, 1, new QTableWidgetItem(scores[i].name));
        lbTable->setItem(i, 2, new QTableWidgetItem(QString::number(scores[i].score)));
        lbTable->setItem(i, 3, new QTableWidgetItem(QString::number(scores[i].level)));
        
        // Center text in all cells
        for (int c = 0; c < 4; c++) {
            if (auto* item = lbTable->item(i, c)) {
                item->setTextAlignment(Qt::AlignCenter);
            }
        }
    }
}

// ── Keyboard Controls ─────────────────────────────────────────
void GameWindow::keyPressEvent(QKeyEvent* e) 
{
    int s = eng->state;
    
    // Enter/Space can start or resume the game
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Space) {
        if (s == STATE_MENU)   { startGame(); return; }
        if (s == STATE_PAUSED) { eng->resume(); return; }
        if (s == STATE_WIN) {
            if (eng->levels.current && eng->levels.current->next) {
                // More levels remain — advance
                eng->nextLevel();
            } else {
                // Final level complete — go straight to the leaderboard
                eng->state = STATE_MENU;
                showLeaderboard();
            }
            return;
        }
    }
    
    // R restarts the game
    if (e->key() == Qt::Key_R && (s == STATE_DEAD || s == STATE_GAMEOVER || s == STATE_WIN)) { 
        eng->start(); 
        return; 
    }
    
    // Esc pauses the game
    if (e->key() == Qt::Key_Escape && s == STATE_PAUSED) { 
        eng->resume(); 
        return; 
    }
    
    // Cheat code: Ctrl+L = extra life
    if (e->key() == Qt::Key_L && (e->modifiers() & Qt::ControlModifier) && s == STATE_PLAYING) {
        eng->lives++;
        emit eng->scoreChanged(eng->score); 
        return;
    }
    
    // Pass other keys to the game engine
    eng->keyPress(e->key());
}

void GameWindow::keyReleaseEvent(QKeyEvent* e) 
{ 
    eng->keyRelease(e->key()); 
}

void GameWindow::closeEvent(QCloseEvent* e) 
{ 
    e->accept(); // Let the window close normally
}
