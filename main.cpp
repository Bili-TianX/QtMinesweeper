#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QPaintEvent>
#include <QRandomGenerator>
#include <QMouseEvent>
#include <QMap>
#include <QtGlobal>
#include <QDebug>

const int ROW_COUNT = 15;
const int COLUMN_COUNT = 15;
const int BLOCK_WIDTH = 32;
const int BLOCK_HEIGHT = 32;
const int BOOM_COUNT = 30;
const int WINDOW_WIDTH = ROW_COUNT * BLOCK_WIDTH;
const int WINDOW_HEIGHT = COLUMN_COUNT * BLOCK_HEIGHT;

enum BLOCK {
    ZERO, ONE, TWO, THREE, FOUR,
    FIVE, SIX, SEVEN, EIGHT, BOOM,
    FLAG, UNKNOWN
};
QMap<BLOCK, QPixmap> map;

inline bool isNum(BLOCK b) {
    return b == ONE || b == TWO || b == THREE ||
           b == FOUR || b == FIVE || b == SIX || b == SEVEN ||
           b == EIGHT;
}

class Window : public QWidget {
public:
    BLOCK data[ROW_COUNT][COLUMN_COUNT];
    BLOCK view[ROW_COUNT][COLUMN_COUNT];
    bool gameOver = false;
    bool youWin = false;
    bool showRect = false;
    int rx = -1, ry = -1;

    Window() {
        setWindowTitle("扫雷");
        setFixedSize(WINDOW_WIDTH, WINDOW_HEIGHT);
        setFont(QFont("微软雅黑", 20));
        // 初始化
        for (int i = 0; i < ROW_COUNT; ++i)
            for (int j = 0; j < COLUMN_COUNT; ++j)
                data[i][j] = ZERO, view[i][j] = UNKNOWN;
        // 生成炸弹
        for (int i = 0; i < BOOM_COUNT; ++i) {
            int v = QRandomGenerator::global()->generate() % (ROW_COUNT * COLUMN_COUNT);
            int x = v / ROW_COUNT, y = v % ROW_COUNT;
            data[x][y] = BOOM;
        }
        // 统计数量
        for (int i = 0; i < ROW_COUNT; ++i)
            for (int j = 0; j < COLUMN_COUNT; ++j) {
                if (data[i][j] == BOOM) continue;

                int sum = 0;
                for (int dx = -1; dx <= 1; ++dx) {
                    for (int dy = -1; dy <= 1; ++dy) {
                        if (i + dx >= 0 && i + dx < ROW_COUNT &&
                            j + dy >= 0 && j + dy < COLUMN_COUNT &&
                            data[i + dx][j + dy] == BOOM) ++sum;
                    }
                }

                switch (sum) {
                case 0: data[i][j] = ZERO; break;
                case 1: data[i][j] = ONE; break;
                case 2: data[i][j] = TWO; break;
                case 3: data[i][j] = THREE; break;
                case 4: data[i][j] = FOUR; break;
                case 5: data[i][j] = FIVE; break;
                case 6: data[i][j] = SIX; break;
                case 7: data[i][j] = SEVEN; break;
                case 8: data[i][j] = EIGHT; break;
                }
            }
    };

    void dfs(int i, int j) {
        if (gameOver || youWin) return;
        if (view[i][j] != UNKNOWN) return;
        view[i][j] = data[i][j];

        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                if (dx == 0 && dy == 0) continue;

                int nx = i + dx, ny = j + dy;
                if (nx >= 0 && nx < ROW_COUNT && ny >= 0 && ny < COLUMN_COUNT) {

                    if (data[nx][ny] == ZERO) dfs(nx, ny);
                    else if (isNum(data[nx][ny])) view[nx][ny] = data[nx][ny];
                }
            }
        }
    }

    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event)

        QPainter painter(this);

        for (int i = 0; i < ROW_COUNT; ++i)
            for (int j = 0; j < COLUMN_COUNT; ++j)
                painter.drawPixmap(BLOCK_WIDTH * i, BLOCK_HEIGHT * j,
                                   gameOver ? map[data[i][j]] : map[view[i][j]]);

        if (showRect) {
            QPen pen;
            pen.setWidth(5);
            pen.setColor(Qt::red);
            painter.setPen(pen);
            painter.drawRect((rx - 1) * BLOCK_WIDTH, (ry - 1) * BLOCK_HEIGHT, BLOCK_WIDTH * 3, BLOCK_HEIGHT * 3);
        }

        if (youWin) {
            painter.fillRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, QColor(255, 0, 0, 100));
            painter.drawText(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Qt::AlignCenter, "You Win!");
        }
    }

    bool Win() {
        for (int i = 0; i < ROW_COUNT; ++i)
            for (int j = 0; j < COLUMN_COUNT; ++j)
                if ((view[i][j] == UNKNOWN || view[i][j] == FLAG) && data[i][j] != BOOM) return false;
        return true;
    }

    void mousePressEvent(QMouseEvent *event) override {
        if (gameOver || youWin) return;

        int x = event->pos().x() / BLOCK_WIDTH;
        int y = event->pos().y() / BLOCK_HEIGHT;

        if (event->button() == Qt::LeftButton && view[x][y] == UNKNOWN) {
            if (data[x][y] == BOOM) {
                gameOver = true;
                repaint();
            }

            dfs(x, y);
            youWin = Win();
            repaint();
        } else if (event->button() == Qt::RightButton && view[x][y] == UNKNOWN) {
            view[x][y] = view[x][y] == FLAG ? UNKNOWN : FLAG;
            repaint();
        } else if (event->button() == Qt::MiddleButton) {
            showRect = true;
            rx = x, ry = y;
            repaint();
        }

    }

    void mouseReleaseEvent(QMouseEvent *event) override {
        if (event->button() == Qt::MiddleButton) {
            showRect = false;
            repaint();
        }
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    auto *tiles = new QPixmap(":/src/tiles.jpg");
    map.insert(ZERO, tiles->copy(0, 0, 32, 32));
    map.insert(ONE, tiles->copy(BLOCK_WIDTH * 1, 0, BLOCK_WIDTH, BLOCK_HEIGHT));
    map.insert(TWO, tiles->copy(BLOCK_WIDTH * 2, 0, BLOCK_WIDTH, BLOCK_HEIGHT));
    map.insert(THREE, tiles->copy(BLOCK_WIDTH * 3, 0, BLOCK_WIDTH, BLOCK_HEIGHT));
    map.insert(FOUR, tiles->copy(BLOCK_WIDTH * 4, 0, BLOCK_WIDTH, BLOCK_HEIGHT));
    map.insert(FIVE, tiles->copy(BLOCK_WIDTH * 5, 0, BLOCK_WIDTH, BLOCK_HEIGHT));
    map.insert(SIX, tiles->copy(BLOCK_WIDTH * 6, 0, BLOCK_WIDTH, BLOCK_HEIGHT));
    map.insert(SEVEN, tiles->copy(BLOCK_WIDTH * 7, 0, BLOCK_WIDTH, BLOCK_HEIGHT));
    map.insert(EIGHT, tiles->copy(BLOCK_WIDTH * 8, 0, BLOCK_WIDTH, BLOCK_HEIGHT));
    map.insert(BOOM, tiles->copy(BLOCK_WIDTH * 9, 0, BLOCK_WIDTH, BLOCK_HEIGHT));
    map.insert(UNKNOWN, tiles->copy(BLOCK_WIDTH * 10, 0, BLOCK_WIDTH, BLOCK_HEIGHT));
    map.insert(FLAG, tiles->copy(BLOCK_WIDTH * 11, 0, BLOCK_WIDTH, BLOCK_HEIGHT));

    Window window;
    window.show();

    return a.exec();
}
