#include "serveurcrawler.h"
#include "ui_serveurcrawler.h"


ServeurCrawler::ServeurCrawler(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ServeurCrawler),
    socketEcoute(new QTcpServer(this)),
    grille(new QGridLayout(this))


{
    ui->setupUi(this);
    //    for(int ligne=0; ligne<TAILLE; ligne++)
    //    {
    //        for (int colonne=0;colonne<TAILLE;colonne++)
    //        {
    //            QToolButton *b = new QToolButton();
    //            grille->addWidget(b,ligne,colonne,1,1);
    //            }
    //    }

    player = new QMediaPlayer();
    audiOutput = new QAudioOutput();
    player->setAudioOutput(audiOutput);

    scene=new QGraphicsScene();
    vue = new QGraphicsView(this);
    scene->setSceneRect(0,0,1948,1178);

    setWindowState(Qt::WindowMaximized);
    tresor=donnerPositionUnique();
    qDebug() << this << "Position du trésor" << tresor;
    afficherGrille();
    vue->setBackgroundBrush(QBrush(QPixmap("/home/USERS/ELEVES/SNIR2021/tsoulaimana/Qt/projet noel/loose/serveur/image/map.png")));
    vue->fitInView(scene->sceneRect());
    vue->setScene(scene);
    vue->setWindowState(Qt::WindowMaximized);
    vue->show();
    grille->addWidget(vue);
    // Placement des labels en bas de la grille
    grille->addWidget(ui->labelPortEcoute,TAILLE,0,1,4);
    //port
    grille->addWidget(ui->spinBoxPortEcoute,TAILLE,5,1,3);
    // connexion et quitter
    grille->addWidget(ui->pushButtonLancementServeur,TAILLE,10,1,4);
    grille->addWidget(ui->pushButtonQuitter,TAILLE,16,1,3);
    this->setLayout(grille);

    setFocus();
}


ServeurCrawler::~ServeurCrawler()
{
    delete ui;
}

void ServeurCrawler::on_pushButtonLancementServeur_clicked()
{
    if(!socketEcoute->listen(QHostAddress::Any,ui->spinBoxPortEcoute->value()))
    {
        QString message = "Impossible de démarrer le serveur " + socketEcoute->errorString();
        qDebug()<<message;
        socketEcoute->close();
    }
    else
    {

        connect(socketEcoute,&QTcpServer::newConnection,this,&ServeurCrawler::onQTcpServer_newConnection);

        ui->pushButtonLancementServeur->setEnabled(false);
        ui->spinBoxPortEcoute->setEnabled(false);
    }

}

void ServeurCrawler::onQTcpServer_newConnection()
{
    QTcpSocket *client;
    client = socketEcoute->nextPendingConnection();
    connect(client, &QTcpSocket::readyRead,   this, &ServeurCrawler::onQTcpSocket_readyRead);
    connect(client, &QTcpSocket::disconnected,this, &ServeurCrawler::onQTcpSocket_disconnected);

    QHostAddress addresseClient = client->peerAddress();

    qDebug() << this << addresseClient.toString();
    listeSocketsClient.append(client);
    QPoint pointDepart=donnerPositionUnique();
    listePositions.append(pointDepart);
    EnvoyerDonnees(client,pointDepart,"start");

}

void ServeurCrawler::onQTcpSocket_readyRead()
{
    QChar   commande;
    quint16 taille=0;
    QTcpSocket *socketVersClient=qobject_cast<QTcpSocket*>(sender());
    int indexClient = listeSocketsClient.indexOf(socketVersClient);
    QPoint pos = listePositions.at(indexClient);

    if (socketVersClient->bytesAvailable() >= (qint64)sizeof(taille))
    {
        QDataStream in(socketVersClient);
        in >> taille;

        if (socketVersClient->bytesAvailable() >= (qint64)taille)
        {
            // decomposition de la trame
            in >> commande;
            switch (commande.toLatin1()) {
            case 'D':
                pos.setY((pos.y()+11));
                break;
            case 'U':
                pos.setY(abs(pos.y()-11));
                break;
            case 'R':
                pos.setX((pos.x()+11));
                break;
            case 'L':
                pos.setX(abs(pos.x()-11));
                break;
            }
            // collision?
            if (listePositions.contains(pos))
            {
                int indexAutre = listePositions.indexOf(pos);
                listePositions.replace(indexClient,donnerPositionUnique());
                listePositions.replace(indexAutre,donnerPositionUnique());
                // envoyer position aux clients
                EnvoyerDonnees(socketVersClient,listePositions.at(indexClient),"collision");
                EnvoyerDonnees(listeSocketsClient.at(indexAutre),listePositions.at(indexAutre),"collision");
            }
            else
            {
                // gagnant?
                if (pos==tresor)
                {
                    qDebug() << this << "un gagnant";
                    // envoyer message à tous les clients
                    foreach(QTcpSocket *client, listeSocketsClient)
                    {
                        EnvoyerDonnees(client,QPoint(-1,-1),"Victoire de "+socketVersClient->peerAddress().toString());
                    }
                    //deconnecter tous les client
                    foreach(QTcpSocket *client, listeSocketsClient)
                    {
                        client->disconnectFromHost();
                        qDebug()<<"deconnexion";
                    }
                    tresor=donnerPositionUnique();
                    viderGrille();
                }
                else
                {
                    // autre
                    listePositions.replace(indexClient,pos);
                    // envoyer position au client
                    EnvoyerDonnees(socketVersClient,pos,"vide");
                }
            }
            afficherGrille();
        }
    }
}

void ServeurCrawler::onQTcpSocket_disconnected()
{
    qDebug() << this << "un client en moins";
    QTcpSocket *socketVersClient=qobject_cast<QTcpSocket*>(sender());
    int indexClient = listeSocketsClient.indexOf(socketVersClient);
    listeSocketsClient.removeAt(indexClient);
    listePositions.removeAt(indexClient);
    disconnect(socketVersClient,&QTcpSocket::readyRead,this,&ServeurCrawler::onQTcpSocket_readyRead);
    disconnect(socketVersClient,&QTcpSocket::disconnected,this,&ServeurCrawler::onQTcpSocket_disconnected);
    // socketVersClient->deleteLater();
    //delete socketVersClient;

    qDebug() <<listePositions.size();
    afficherGrille();

}

void ServeurCrawler::EnvoyerDonnees(QTcpSocket *client, QPoint pt, QString msg)
{
    quint16 taille=0;

    QBuffer tampon;
    tampon.open(QIODevice::WriteOnly);
    // association du tampon au flux de sortie
    QDataStream out(&tampon);
    // construction de la trame
    out << taille << pt << msg << calculerDistance(pt);
    // calcul de la taille de la trame
    taille = tampon.size()-sizeof(taille);

    // placement sur la premiere position du flux pour pouvoir modifier la taille
    tampon.seek(0);
    //modification de la trame avec la taille reel de la trame
    out << taille;
    // envoi du QByteArray du tampon via la socket
    client->write(tampon.buffer());
}

void ServeurCrawler::afficherGrille()
{
    viderGrille();
    foreach(QPoint pt, listePositions)
    {
        QPolygon polyPerso;
        polyPerso.setPoints(4,0,0,0,15,15,15,15,0);
        QGraphicsPolygonItem *personnage= new QGraphicsPolygonItem(polyPerso);
        QBrush fondPixMap;
        QPixmap monpixmap("../son.png");
        vue = new QGraphicsView(this);
        fondPixMap.setTexture(monpixmap);
        personnage->setBrush(fondPixMap);
        scene->addItem(personnage);
    }
    //    grille->itemAtPosition(tresor.y(),tresor.x())->widget()->setStyleSheet("background-color : red");
    QGraphicsPixmapItem *rectTresor = new QGraphicsPixmapItem(QPixmap("/home/USERS/ELEVES/SNIR2021/tcorbillon/Bureau/Deuxieme année/C++/projet noel/prog prof/serveur/tresors.png"));
    rectTresor->setPos(tresor);
    scene->addItem(rectTresor);


}

void ServeurCrawler::viderGrille()
{
    scene->clear();

}

QPoint ServeurCrawler::donnerPositionUnique()
{
    QRandomGenerator gen;
    QPoint pt;
    gen.seed(QDateTime::currentMSecsSinceEpoch());
    int ligne;
    int colonne;
    do
    {
        do{

            ligne=gen.bounded(1178);
            attendre(20);
        }while(ligne % 11 != 0);
        do{
            colonne=gen.bounded(1948);
            attendre(20);
        }while(colonne % 11 != 0);

        pt=QPoint(colonne,ligne);

    }while (listePositions.contains(pt));

    return pt;
}

/**
 * @brief ServeurCrawler::calculerDistance
 * @param pos
 * @return la distance entre le joueur et le trésor
 */
double ServeurCrawler::calculerDistance(QPoint pos)
{
    QPoint d = tresor - pos;
    return sqrt(d.x()*d.x() + d.y()* d.y());
}

/**
 * @brief ServeurCrawler::attendre
 * @param millisecondes le temps d'attente
 * @details introduit un temps d'attente
 */
void ServeurCrawler::attendre(int millisecondes)
{
    QTimer timer;
    timer.start(millisecondes);
    QEventLoop loop(this);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    loop.exec();
}

