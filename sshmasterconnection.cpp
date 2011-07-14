/*
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "x2goclientconfig.h"
#include "x2gologdebug.h"
#include "sshmasterconnection.h"
#include <stdlib.h>
#include <stdio.h>
#include "sshprocess.h"

#include <QStringList>
#include <QFile>
#include <QDir>
#include <QTemporaryFile>
#ifndef Q_OS_WIN
#include <arpa/inet.h>
#endif

#include "onmainwindow.h"

#undef DEBUG
// #define DEBUG

static bool isLibSshInited=false;

SshMasterConnection::SshMasterConnection ( QString host, int port, bool acceptUnknownServers, QString user,
        QString pass, QString key,bool autologin, QObject* parent ) : QThread ( parent )
{
    this->host=host;
    this->port=port;
    this->user=user;
    this->pass=pass;
    this->key=key;
    this->autologin=autologin;
    this->acceptUnknownServers=acceptUnknownServers;
    reverseTunnel=false;
    mainWnd=(ONMainWindow*) parent;
}

SshMasterConnection::SshMasterConnection ( QString host, int port, bool acceptUnknownServers, QString user,
        QString pass, QString key, bool autologin,
        int remotePort, QString localHost, int localPort, SshProcess* creator, 
					   QObject* parent, ONMainWindow* mwd ) : QThread ( parent )
{

    this->host=host;
    this->port=port;
    this->user=user;
    this->pass=pass;
    this->key=key;
    this->autologin=autologin;
    this->acceptUnknownServers=acceptUnknownServers;
    reverseTunnelLocalHost=localHost;
    reverseTunnelLocalPort=localPort;
    reverseTunnelCreator=creator;
    reverseTunnel=true;
    reverseTunnelRemotePort=remotePort;
    mainWnd=mwd;
}

SshMasterConnection* SshMasterConnection::reverseTunnelConnection ( SshProcess* creator,
        int remotePort, QString localHost, int localPort )
{
    SshMasterConnection* con=new SshMasterConnection ( host,port,acceptUnknownServers,user,pass,
            key,autologin, remotePort,localHost,
            localPort,creator,this, mainWnd);

    connect ( con,SIGNAL ( ioErr ( SshProcess*,QString,QString ) ),this,SIGNAL ( ioErr ( SshProcess*,QString,QString ) ) );
    connect ( con,SIGNAL ( stdErr ( SshProcess*,QByteArray ) ),this,SIGNAL ( stdErr ( SshProcess*,QByteArray ) ) );
    connect ( con,SIGNAL ( reverseListenOk ( SshProcess* ) ), this, SIGNAL ( reverseListenOk ( SshProcess* ) ) );
    con->start();
    reverseTunnelConnectionsMutex.lock();
    reverseTunnelConnections.append ( con );
    reverseTunnelConnectionsMutex.unlock();
    return con;
}

void SshMasterConnection::run()
{
    disconnectSessionFlag=false;
    if ( !isLibSshInited )
    {
#ifdef DEBUG
        x2goDebug<<"libSsh not inited yet, initting\n";
#endif
        if ( ssh_init() !=0 )
        {
            QString err=tr ( "Can not initialize libssh" );
            x2goDebug<<err<<endl;
            emit connectionError ( err,"" );
            quit();
            return;
        }
        isLibSshInited=true;
    }
#ifdef DEBUG
    else
    {
        x2goDebug<<"already inited\n";
    }
#endif

//     int verbosity=SSH_LOG_PROTOCOL;

    my_ssh_session = ssh_new();
    if ( my_ssh_session == NULL )
    {
        QString err=tr ( "Can not create ssh session" );
        x2goDebug<<err<<endl;
        emit connectionError ( err,"" );
        if ( reverseTunnel )
            emit ioErr ( reverseTunnelCreator,err,"" );
        quit();
        return;
    }
#ifdef Q_OS_WIN    
    ssh_options_set ( my_ssh_session, SSH_OPTIONS_SSH_DIR, (mainWnd->getHomeDirectory()+"/ssh").toAscii());
#endif
//     ssh_options_set(my_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
    if ( !sshConnect() )
    {
        QString err=ssh_get_error ( my_ssh_session );
        QString message=tr ( "Can not connect to " ) +host+QString::number ( port );
        x2goDebug<<message<<" - "<<err;
        emit connectionError ( message, err );
        if ( reverseTunnel )
            emit ioErr ( reverseTunnelCreator,message,err );
        ssh_free ( my_ssh_session );
        quit();
        return;
    }
    QString errMsg;
    int state=serverAuth ( errMsg );
    if ( state != SSH_SERVER_KNOWN_OK )
    {
        emit serverAuthError ( state,errMsg );
        ssh_disconnect ( my_ssh_session );
        ssh_free ( my_ssh_session );
        quit();
        return;
    }

    ssh_options_set ( my_ssh_session, SSH_OPTIONS_USER, user.toAscii() );
#ifdef Q_OS_WIN    
    ssh_options_set ( my_ssh_session, SSH_OPTIONS_SSH_DIR, (mainWnd->getHomeDirectory()+"/ssh").toAscii());
#endif    
    x2goDebug<<"setting SSH DIR to "<<mainWnd->getHomeDirectory()+"/ssh";

    if ( userAuth() )
    {
      #ifdef DEBUG
        x2goDebug<<"user auth OK\n";
      #endif
        emit connectionOk();
    }
    else
    {
        QString err=ssh_get_error ( my_ssh_session );
        QString message=tr ( "Authentication failed" );
        x2goDebug<<message<<" - "<<err;
        emit userAuthError ( authErrors.join ( "\n" ) );
        if ( reverseTunnel )
            emit ioErr ( reverseTunnelCreator,message,err );
        ssh_disconnect ( my_ssh_session );
        ssh_free ( my_ssh_session );
        quit();
        return;
    }

    if ( reverseTunnel )
    {
        if ( channel_forward_listen ( my_ssh_session, NULL, reverseTunnelRemotePort,  NULL ) !=SSH_OK )
        {
            QString err=ssh_get_error ( my_ssh_session );
            QString message=tr ( "channel_forward_listen failed" );
            x2goDebug<<message<<" - "<<err;
            emit ioErr ( reverseTunnelCreator, message, err );
            ssh_disconnect ( my_ssh_session );
            ssh_free ( my_ssh_session );
            quit();
            return;
        }
        emit reverseListenOk ( reverseTunnelCreator );

#ifdef DEBUG
        x2goDebug<<"channel_forward_listen ok\n ";
#endif
    }
    channelLoop();
}


SshMasterConnection::~SshMasterConnection()
{
    /*    ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);*/
}


void SshMasterConnection::finalizeLibSsh()
{
    if ( !isLibSshInited )
    {
#ifdef DEBUG
        x2goDebug<<"libssh not inited yet\n";
#endif
        return;
    }

    ssh_finalize();
#ifdef DEBUG
    x2goDebug<<"libssh finalized\n";
#endif
}

bool SshMasterConnection::sshConnect()
{
    int rc;
    ssh_options_set ( my_ssh_session, SSH_OPTIONS_HOST, host.toAscii() );
    ssh_options_set ( my_ssh_session, SSH_OPTIONS_PORT, &port );
    rc = ssh_connect ( my_ssh_session );
    if ( rc != SSH_OK )
        return false;
    return true;

}

int SshMasterConnection::serverAuth ( QString& errorMsg )
{
#ifdef DEBUG
    x2goDebug<<"cserverAuth\n ";
#endif

    int state, hlen;
    unsigned char *hash = NULL;
    char *hexa;

    state = ssh_is_server_known ( my_ssh_session );
    hlen = ssh_get_pubkey_hash ( my_ssh_session, &hash );

    if ( hlen < 0 )
        return SSH_SERVER_ERROR;

#ifdef DEBUG
    x2goDebug<<"state: "<<state<<endl;
#endif


    switch ( state )
    {
    case SSH_SERVER_KNOWN_OK:
        break; /* ok */

    case SSH_SERVER_KNOWN_CHANGED:
        hexa = ssh_get_hexa ( hash, hlen );
        errorMsg=hexa;
        free ( hexa );
        break;
    case SSH_SERVER_FOUND_OTHER:
        break;
    case SSH_SERVER_FILE_NOT_FOUND:
    case SSH_SERVER_NOT_KNOWN:
        if ( !acceptUnknownServers )
        {
            hexa = ssh_get_hexa ( hash, hlen );
            errorMsg=hexa;
            free ( hexa );
            break;
        }
        ssh_write_knownhost ( my_ssh_session );
        state=SSH_SERVER_KNOWN_OK;
        break;

    case SSH_SERVER_ERROR:
        errorMsg=ssh_get_error ( my_ssh_session );
        break;
    }
    free ( hash );
    return state;
}


bool SshMasterConnection::userAuthWithPass()
{
    int rc = ssh_userauth_password ( my_ssh_session, NULL, pass.toAscii() );
    if ( rc != SSH_AUTH_SUCCESS )
    {
        QString err=ssh_get_error ( my_ssh_session );
        authErrors<<err;
#ifdef DEBUG
        x2goDebug<<"userAuthWithPass failed:" <<err<<endl;
#endif
        return false;
    }
    return true;
}


bool SshMasterConnection::userAuthAuto()
{
    int rc = ssh_userauth_autopubkey ( my_ssh_session, NULL );
    if ( rc != SSH_AUTH_SUCCESS )
    {
        QString err=ssh_get_error ( my_ssh_session );
        authErrors<<err;
#ifdef DEBUG
        x2goDebug<<"userAuthAuto failed:" <<err<<endl;
#endif
        return false;
    }
    return true;
}

bool SshMasterConnection::userAuthWithKey()
{
#ifdef DEBUG
    x2goDebug<<"try authenticate user with private key" <<endl;
#endif
    QString keyName=key;
    bool autoRemove=false;
    if ( key.indexOf ( "PRIVATE KEY" ) !=-1 )
    {
        QDir dr;
        QString keyPath=mainWnd->getHomeDirectory() +"/.x2go/ssh/gen";
        dr.mkpath ( keyPath );
        QTemporaryFile fl ( keyPath+"/key" );
        fl.open();
        keyName=fl.fileName();
        fl.setAutoRemove ( false );
        QTextStream out ( &fl );
        out<<key;
        fl.close();
        autoRemove=true;
#ifdef DEBUG
        x2goDebug<<"temporary save key in "<<keyName<<endl;
#endif
    }

    ssh_private_key prkey=privatekey_from_file(my_ssh_session, keyName.toAscii(), 0, pass.toAscii());
    if (!prkey)
    {
#ifdef DEBUG
        x2goDebug<<"Failed to get private key from "<<keyName<<endl;
#endif
        if ( autoRemove )
            QFile::remove ( keyName );
        return false;
    }
    ssh_public_key pubkey=publickey_from_privatekey(prkey);
    if (!prkey)
    {
#ifdef DEBUG
        x2goDebug<<"Failed to get public key from private key"<<endl;
#endif
        privatekey_free(prkey);
        if ( autoRemove )
            QFile::remove ( keyName );
        return false;
    }

    ssh_string pubkeyStr=publickey_to_string(pubkey);
    publickey_free(pubkey);

    //not implemented before libssh 0.5
    /*	int rc = ssh_userauth_privatekey_file ( my_ssh_session,NULL,
    	                                        keyName.toAscii(),
    	                                        pass.toAscii() );*/

    int rc=ssh_userauth_pubkey(my_ssh_session, NULL, pubkeyStr, prkey);
    privatekey_free(prkey);
    string_free(pubkeyStr);

#ifdef DEBUG
    x2goDebug<<"auth with key: "<<rc<<endl;
#endif

    if ( autoRemove )
        QFile::remove ( keyName );
    if ( rc != SSH_AUTH_SUCCESS )
    {
        QString err=ssh_get_error ( my_ssh_session );
        authErrors<<err;

#ifdef DEBUG
        x2goDebug<<"userAuthWithKey failed:" <<err<<endl;
#endif

        return false;
    }
    return true;
}


bool SshMasterConnection::userAuth()
{
    if ( autologin )
        if ( userAuthAuto() )
            return true;
    if ( key!="" )
    {
        if ( userAuthWithKey() )
            return true;
    }
    return userAuthWithPass();
}


void SshMasterConnection::addChannelConnection ( SshProcess* creator, int sock, QString forwardHost, int forwardPort,
        QString localHost, int localPort, void* channel )
{
    ChannelConnection con;
    con.channel= ( ssh_channel ) channel;
    con.sock=sock;
    con.creator=creator;
    con.forwardHost=forwardHost;
    con.forwardPort=forwardPort;
    con.localHost=localHost;
    con.localPort=localPort;

    channelConnectionsMutex.lock();
    channelConnections<<con;
    channelConnectionsMutex.unlock();
}


void SshMasterConnection::addChannelConnection ( SshProcess* creator, QString cmd )
{
    ChannelConnection con;
    con.channel=NULL;
    con.sock=-1;
    con.creator=creator;
    con.command=cmd;

    channelConnectionsMutex.lock();
    channelConnections<<con;
    channelConnectionsMutex.unlock();
}


void SshMasterConnection::addCopyRequest ( SshProcess* creator, QString src, QString dst )
{
    CopyRequest req;
    req.src=src;
    req.dst=dst;
    req.creator=creator;
    copyRequestMutex.lock();
    copyRequests<<req;
    copyRequestMutex.unlock();
}

void SshMasterConnection::disconnectSession()
{
    disconnectFlagMutex.lock();
    disconnectSessionFlag=true;
    disconnectFlagMutex.unlock();
}

void SshMasterConnection::copy()
{

    for ( int i=copyRequests.size()-1;i>=0;--i )
    {
        QStringList lst=copyRequests[i].dst.split ( "/" );
        QString dstFile=lst.last();
        lst.removeLast();
        QString dstPath=lst.join ( "/" );
#ifdef DEBUG
        x2goDebug<<"dst path:"<<dstPath<<" file:"<<dstFile<<endl;
#endif
        ssh_scp scp=ssh_scp_new ( my_ssh_session, SSH_SCP_WRITE|SSH_SCP_RECURSIVE, dstPath.toAscii() );
        if ( scp == NULL )
        {
            x2goDebug<<"Error allocating scp session: "<< ssh_get_error ( my_ssh_session ) <<endl;
            return;
        }
        int rc = ssh_scp_init ( scp );
        if ( rc != SSH_OK )
        {
            x2goDebug<<"Error initializing scp session: "<< ssh_get_error ( my_ssh_session ) <<endl;
            ssh_scp_free ( scp );
            return;
        }
        QFile file ( copyRequests[i].src );
        if ( !file.open ( QIODevice::ReadOnly ) )
        {
            QString errMsg=tr ( "Can not open file " ) +copyRequests[i].src;
            emit copyErr ( copyRequests[i].creator, errMsg, "" );
            copyRequests.removeAt ( i );
            ssh_scp_close ( scp );
            ssh_scp_free ( scp );
            continue;
        }
        QByteArray arr=file.readAll();
        file.close();
        rc=ssh_scp_push_file ( scp,dstFile.toAscii(),arr.size(), 0600 );
        if ( rc != SSH_OK )
        {
            QString errMsg=tr ( "Can not create remote file " ) +copyRequests[i].dst;
            QString serr=ssh_get_error ( my_ssh_session );
            x2goDebug<<errMsg<<" - "<<serr<<endl;
            emit copyErr ( copyRequests[i].creator, errMsg, serr );
            copyRequests.removeAt ( i );
            ssh_scp_close ( scp );
            ssh_scp_free ( scp );
            continue;
        }
        rc=ssh_scp_write ( scp,arr.data(),arr.size() );
        if ( rc != SSH_OK )
        {
            QString serr=ssh_get_error ( my_ssh_session );
            QString errMsg=tr ( "Can not write to remote file " ) +copyRequests[i].dst;
            x2goDebug<<errMsg<<" - "<<serr<<endl;
            emit copyErr ( copyRequests[i].creator, errMsg, serr );
            copyRequests.removeAt ( i );
            ssh_scp_close ( scp );
            ssh_scp_free ( scp );
            continue;
        }
        emit copyOk ( copyRequests[i].creator );
#ifdef DEBUG
        x2goDebug<<"scp ok: "<<copyRequests[i].src<<" -> "<<user<<"@"<<host<<":"<<copyRequests[i].dst<<endl;
#endif
        copyRequests.removeAt ( i );
        ssh_scp_close ( scp );
        ssh_scp_free ( scp );
    }
}

void SshMasterConnection::channelLoop()
{
    forever
    {
        copyRequestMutex.lock();
        if ( copyRequests.size() >0 )
            copy();
        copyRequestMutex.unlock();
        if ( reverseTunnel )
        {
            ssh_channel newChan=channel_forward_accept ( my_ssh_session,0 );
            if ( newChan )
            {
#ifdef DEBUG
                x2goDebug<<"new forward connection"<<endl;
#endif
                int sock=socket ( AF_INET, SOCK_STREAM,0 );
                struct sockaddr_in address;
                address.sin_family=AF_INET;
                address.sin_port=htons ( reverseTunnelLocalPort );
#ifdef DEBUG
                x2goDebug<<"connecting to "<<reverseTunnelLocalHost<<":"<<reverseTunnelLocalPort<<endl;
#endif
#ifndef Q_OS_WIN
                inet_aton ( reverseTunnelLocalHost.toAscii(), &address.sin_addr );
#else
                address.sin_addr.s_addr=inet_addr (
                                            reverseTunnelLocalHost.toAscii() );
#endif

                if ( ::connect ( sock, ( struct sockaddr * ) &address,sizeof ( address ) ) !=0 )
                {
                    QString errMsg=tr ( "can not connect to " ) +
                                   reverseTunnelLocalHost+":"+QString::number ( reverseTunnelLocalPort );
                    x2goDebug<<errMsg<<endl;
                    emit ioErr ( reverseTunnelCreator, errMsg, "" );
                    continue;
                }
#ifdef DEBUG
                x2goDebug<<"creating new channel connection"<<endl;
#endif
                ChannelConnection con;
                con.channel=newChan;
                con.sock=sock;
                con.creator=reverseTunnelCreator;
                channelConnectionsMutex.lock();
                channelConnections<<con;
                channelConnectionsMutex.unlock();
            }
        }

        char buffer[1024*512]; //512K buffer
        int nbytes;
        fd_set rfds;

        struct timeval tv;

        tv.tv_sec = 0;
        tv.tv_usec = 500;

        int retval;
        int maxsock=-1;

        disconnectFlagMutex.lock();
        bool disconnect=disconnectSessionFlag;
        disconnectFlagMutex.unlock();

        if ( disconnect )
        {
#ifdef DEBUG
            if ( !reverseTunnel )
                x2goDebug<<"Disconnecting..."<<endl;
#endif
            reverseTunnelConnectionsMutex.lock();
            for ( int i=0;i<reverseTunnelConnections.size();++i )
            {
                reverseTunnelConnections[i]->disconnectSession();
                reverseTunnelConnections[i]->wait ( 10000 );

            }
            reverseTunnelConnectionsMutex.unlock();

            channelConnectionsMutex.lock();
            for ( int i=0;i<channelConnections.size();++i )
            {
                finalize ( i );
            }
            channelConnectionsMutex.unlock();

            ssh_disconnect ( my_ssh_session );
            ssh_free ( my_ssh_session );
#ifdef DEBUG
            if ( !reverseTunnel )
                x2goDebug<<"All channels closed, session disconnected, quiting session loop"<<endl;
#endif
            quit();
            return;
        }


        channelConnectionsMutex.lock();
        if ( channelConnections.size() <=0 )
        {
//             x2goDebug<<"no channel connections, waiting\n";
            channelConnectionsMutex.unlock();
            usleep ( 500 );
            continue;
        }
        ssh_channel* read_chan=new ssh_channel[channelConnections.size() +1];
        ssh_channel* out_chan=new ssh_channel[channelConnections.size() +1];
        read_chan[channelConnections.size() ]=NULL;

        FD_ZERO ( &rfds );

        for ( int i=0;i<channelConnections.size();++i )
        {
            int tcpSocket=channelConnections.at ( i ).sock;
            if ( tcpSocket>0 )
                FD_SET ( tcpSocket, &rfds );
            if ( channelConnections.at ( i ).channel==0l )
            {
#ifdef DEBUG
                x2goDebug<<"creating new channel"<<endl;
#endif
                ssh_channel channel=channel_new ( my_ssh_session );
#ifdef DEBUG
                x2goDebug<<"new channel:"<<channel<<endl;
#endif
                channelConnections[i].channel=channel;
                if ( tcpSocket>0 )
                {
#ifdef DEBUG
                    x2goDebug<<"forwarding new channel, local port: "<<channelConnections.at ( i ).localPort<<endl;
#endif
                    if ( channel_open_forward ( channel,
                                                channelConnections.at ( i ).forwardHost.toAscii(),
                                                channelConnections.at ( i ).forwardPort,
                                                channelConnections.at ( i ).localHost.toAscii(),
                                                channelConnections.at ( i ).localPort ) != SSH_OK )
                    {
                        QString err=ssh_get_error ( my_ssh_session );
                        QString errorMsg=tr ( "channel_open_forward failed" );
                        emit ioErr ( channelConnections[i].creator, errorMsg, err );
                        x2goDebug<<errorMsg<<": "<<err<<endl;
                    }
#ifdef DEBUG
                    else
                    {
                        x2goDebug<<" new channel forwarded"<<endl;
                    }
#endif
                }
                else
                {
#ifdef DEBUG
                    x2goDebug<<"executing remote: "<<channelConnections.at ( i ).command<<endl;
#endif
                    if ( channel_open_session ( channel ) !=SSH_OK )
                    {
                        QString err=ssh_get_error ( my_ssh_session );
                        QString errorMsg=tr ( "channel_open_session failed" );
                        emit ioErr ( channelConnections[i].creator, errorMsg, err );
                        x2goDebug<<errorMsg<<": "<<err<<endl;
                    }
                    else if ( channel_request_exec ( channel, channelConnections[i].command.toAscii() ) != SSH_OK )
                    {
                        QString err=ssh_get_error ( my_ssh_session );
                        QString errorMsg=tr ( "channel_request_exec failed" );
                        emit ioErr ( channelConnections[i].creator, errorMsg, err );
                        x2goDebug<<errorMsg<<": "<<err<<endl;
                    }
#ifdef DEBUG
                    else
                    {
                        x2goDebug<<" new exec channel created"<<endl;
                    }
#endif
                }

            }
            read_chan[i]=channelConnections.at ( i ).channel;
            if ( tcpSocket>maxsock )
                maxsock=tcpSocket;
        }
        channelConnectionsMutex.unlock();
        retval=ssh_select ( read_chan,out_chan,maxsock+1,&rfds,&tv );
        delete [] read_chan;
        delete [] out_chan;

        if ( retval == -1 )
        {
            x2goDebug<<"select error\n";
            continue;
        }

//         x2goDebug<<"select exited"<<endl;

        channelConnectionsMutex.lock();
        for ( int i=channelConnections.size()-1;i>=0;--i )
        {
            int tcpSocket=channelConnections.at ( i ).sock;
            ssh_channel channel=channelConnections.at ( i ).channel;
            if ( channel==0l )
                continue;
            if ( channel_poll ( channel,1 ) >0 )
            {
//              x2goDebug<<"read err data from channel\n";
                nbytes = channel_read ( channel, buffer, sizeof ( buffer )-1, 1 );
                emit stdErr ( channelConnections[i].creator, QByteArray ( buffer,nbytes ) );
//              x2goDebug<<nbytes<<" err from channel"<<endl;
            }
            int rez=channel_poll ( channel,0 );
            if ( rez==SSH_EOF )
            {
#ifdef DEBUG
                x2goDebug<<"EOF ON CHANNEL "<<channel<<endl;
#endif
                //////Finished////////
                finalize ( i );
                continue;
            }
            if ( rez>0 )
            {
//                  x2goDebug<<"read data from channel "<<channel<<endl;
                nbytes = channel_read ( channel, buffer, sizeof ( buffer )-1, 0 );
//                  x2goDebug<<nbytes<<" from channel "<<channel<<endl;
                if ( nbytes > 0 )
                {
                    if ( tcpSocket>0 )
                    {
                        if ( send ( tcpSocket,buffer, nbytes,0 ) != nbytes )
                        {
                            QString errMsg=tr ( "error writing to socket" );
                            x2goDebug<<"error writing "<<nbytes<<" to tcp socket"<<tcpSocket<<endl;
                            emit ioErr ( channelConnections[i].creator,errMsg,"" );
                            finalize ( i );
                            continue;
                        }
//                      x2goDebug<<"wrote "<<nbytes<<" to tcp socket "<<tcpSocket<<endl;
                    }
                    else
                    {
                        emit stdOut ( channelConnections[i].creator, QByteArray ( buffer,nbytes ) );
                    }
                }

                if ( nbytes < 0 )
                {
                    //////ERROR!!!!!////////
                    QString err=ssh_get_error ( my_ssh_session );
                    QString errorMsg=tr ( "error reading channel" );
                    emit ioErr ( channelConnections[i].creator, errorMsg, err );
                    x2goDebug<<errorMsg<<" - "<<err<<endl;
                    finalize ( i );
                    continue;
                }

                if ( channel_is_eof ( channel ) )
                {
#ifdef DEBUG
                    x2goDebug<<"EOF ON CHANNEL "<<channel<<endl;
#endif
                    //////Finished////////
                    finalize ( i );
                    continue;
                }
            }
            if ( tcpSocket<=0 )
            {
                continue;
            }
            if ( FD_ISSET ( tcpSocket,&rfds ) )
            {
                nbytes = recv ( tcpSocket, buffer, sizeof ( buffer )-1,0 );
//                  x2goDebug<<nbytes<<" bytes from tcp socket "<<tcpSocket<<endl;
                if ( nbytes > 0 )
                {
                    if ( channel_write ( channel, buffer, nbytes ) !=nbytes )
                    {
                        QString err=ssh_get_error ( my_ssh_session );
                        QString errorMsg=tr ( "channel_write failed" );
                        emit ioErr ( channelConnections[i].creator, errorMsg, err );
                        x2goDebug<<errorMsg<<" - "<<err<<endl;
                        finalize ( i );
                        continue;
                    }
//                      x2goDebug<<nbytes<<" bytes wrote to channel"<<channel<<endl;
                }
                if ( nbytes < 0 )
                {
                    //////ERROR!!!!!////////
                    QString err="";
                    QString errorMsg=tr ( "error reading tcp socket" );
                    emit ioErr ( channelConnections[i].creator, errorMsg, err );
                    x2goDebug<<errorMsg<<" - "<<err<<endl;
                    finalize ( i );
                    continue;
                }
                if ( nbytes==0 )
                {
#ifdef DEBUG
                    x2goDebug<<"socket closed "<<tcpSocket<<endl;
#endif
                    finalize ( i );
                    continue;
                }
            }
        }
        channelConnectionsMutex.unlock();
    }
}

void SshMasterConnection::finalize ( int item )
{
    int tcpSocket=channelConnections.at ( item ).sock;
    ssh_channel channel=channelConnections.at ( item ).channel;
    if ( channel )
    {
        channel_send_eof ( channel );
#ifdef DEBUG
        x2goDebug<<"eof sent\n";
#endif
        channel_close ( channel );
#ifdef DEBUG
        x2goDebug<<"channel closed\n";
#endif
        channel_free ( channel );
    }
    if ( tcpSocket>0 )
    {
        close ( tcpSocket );
    }
    SshProcess* proc=channelConnections[item].creator;
    channelConnections.removeAt ( item );
    emit channelClosed ( proc );
}
