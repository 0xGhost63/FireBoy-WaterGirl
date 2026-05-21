#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QTableWidget>
#include <QLineEdit>
#include "../include/GameEngine.h"
#include "../include/GameRenderer.h"
#include <QMediaPlayer>
#include <QtGlobal>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QMediaPlaylist>
#else
#include <QAudioOutput>
#endif

// Forward-declare the generated UI class
namespace Ui { class GameWindowUI; }

class GameWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit GameWindow(QWidget* parent = nullptr);
    ~GameWindow();
protected:
    void keyPressEvent  (QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;
    void closeEvent     (QCloseEvent* e) override;
private slots:
    void onFrameReady();
    void onStateChanged(int s);
    void showMenu();
    void showLeaderboard();
    void startGame();
    void submitName();
    void playBGM();  // starts background music after event loop is ready
private:
    Ui::GameWindowUI* ui;  // auto-generated from GameWindow.ui
    GameEngine*    eng;
    GameRenderer*  renderer;

    QString currentPlayerName;

    QMediaPlayer* bgMusic;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QMediaPlaylist* playlist;
#else
    QAudioOutput* audioOutput;
#endif

    // Leaderboard data (sorted with QuickSort, searched with BinarySearch)
    ScoreEntry scores[MAX_SCORES];
    int        scoreCount;

    void setupConnections();
    void refreshLeaderboard();
    void saveScore(const QString& name, int score, int level);
    void loadScores();

    // DSA: Screen Stack — tracks navigation history for back-button support
    ScreenStack screenHistory;
    void navigateTo(int screenIndex);  // push current screen, switch to new one
    void navigateBack();               // pop stack, switch to previous screen
};

