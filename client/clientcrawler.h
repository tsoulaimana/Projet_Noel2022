#ifndef CLIENTCRAWLER_H
#define CLIENTCRAWLER_H

#include <QMediaPlayer>
#include <QAudioOutput>
#include <QWidget>
#include <QTcpSocket>
#include <QGridLayout>
#include <QToolButton>
#include <QBuffer>
#include <QHostAddress>
#include <QKeyEvent>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QRandomGenerator>
#include <QDateTime>
#include <QThread>
#include <QPixmap>

#define TAILLE 20

namespace Ui {
class ClientCrawler;
}

class ClientCrawler : public QWidget
{
    Q_OBJECT

public:
    explicit ClientCrawler(QWidget *parent = nullptr);
    ~ClientCrawler();

    void ViderGrille();
    void EnvoyerCommande(QChar commande);

private slots:
    void on_pushButtonConnexion_clicked();

    void on_pushButtonUp_clicked();

    void on_pushButtonLeft_clicked();

    void on_pushButtonRight_clicked();

    void on_pushButtonDown_clicked();
    void 	onQTcpSocket_connected ();
      void 	onQTcpSocket_disconnected ();
      void 	onQTcpSocket_error ( QAbstractSocket::SocketError socketError );
      void 	onQTcpSocket_hostFound ();
      void 	onQTcpSocket_stateChanged ( QAbstractSocket::SocketState socketState );
      void 	onQTcpSocket_aboutToClose ();
      void 	onQTcpSocket_bytesWritten ( qint64 bytes );
      void 	onQTcpSocket_readChannelFinished ();
      void 	onQTcpSocket_readyRead ();

      void keyPressEvent(QKeyEvent *event);

private:

    Ui::ClientCrawler *ui;
    QTcpSocket *socketDeDialogueAvecServeur;
    QGridLayout *grille;
    QPoint position;
    QMediaPlayer *player;
    QAudioOutput *audiOutput;
    QGraphicsView *vue;
    QGraphicsScene *scene;
};

#endif // CLIENTCRAWLER_H
