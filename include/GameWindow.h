#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QTableWidget>
#include "../include/GameEngine.h"
#include "../include/GameRenderer.h"

class GameWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit GameWindow(QWidget* parent = nullptr);
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
private:
    GameEngine*    eng;
    GameRenderer*  renderer;
    QStackedWidget* stack;
    QTableWidget*  lbTable;
    class QLineEdit* nameInput; // Forward declaration or include QLineEdit
    QString currentPlayerName;

    // Leaderboard data (DSA: sorted with QuickSort/BubbleSort)
    ScoreEntry scores[MAX_SCORES];
    int        scoreCount;

    void buildUI();
    void refreshLeaderboard();
    void saveScore(const QString& name, int score, int level, float time);
    void loadScores();
};
