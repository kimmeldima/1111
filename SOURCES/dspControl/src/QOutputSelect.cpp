#include <QAbstractItemView>
#include <QDebug>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QStandardPaths>
#include <QString>
#include <QMessageBox>
#include <QInputDialog>

#include "QOutputSelect.hpp"
#include "ui_QOutputSelect.h"

using namespace Vektorraum;


QOutputSelect::QOutputSelect( uint32_t selection, uint16_t outputaddr, CFreeDspAurora* ptrdsp, QWidget *parent) :
  QDspBlock(parent), ui(new Ui::QOutputSelect)
{
  addr[kOutput] = outputaddr;
  dsp = ptrdsp;

  ui->setupUi(this);

  ui->lineEditResponseFile->installEventFilter( this );

  fileName = "";
  refSpl = 80.0;

  //QListView * listView = new QListView(ui->comboBoxInput );

  //listView->setStyleSheet( "QWidget:item:selected { border: 0px solid #999900; background: transparent; } QWidget:item:checked { font-weight: bold;}" );
  //ui->comboBoxInput->setView(listView);

  /*ui->comboBoxInput->blockSignals( true );
  ui->comboBoxInput->addItem( "Analog 1", 0 );
  ui->comboBoxInput->addItem( "Analog 2", 1 );
  ui->comboBoxInput->addItem( "Analog 3", 2 );
  ui->comboBoxInput->addItem( "Analog 4", 3 );
  ui->comboBoxInput->addItem( "Analog 5", 4 );
  ui->comboBoxInput->addItem( "Analog 6", 5 );
  ui->comboBoxInput->addItem( "Analog 7", 6 );
  ui->comboBoxInput->addItem( "Analog 8", 7 );
  ui->comboBoxInput->addItem( "USB 1", 8 );
  ui->comboBoxInput->addItem( "USB 2", 9 );
  ui->comboBoxInput->addItem( "USB 3", 10 );
  ui->comboBoxInput->addItem( "USB 4", 11 );
  ui->comboBoxInput->addItem( "USB 5", 12 );
  ui->comboBoxInput->addItem( "USB 6", 13 );
  ui->comboBoxInput->addItem( "USB 7", 14 );
  ui->comboBoxInput->addItem( "USB 8", 15 );
  ui->comboBoxInput->setCurrentIndex( selection );
  ui->comboBoxInput->blockSignals( false );*/
  //ui->comboBoxInput->setMaxVisibleItems( 16 );
}

QOutputSelect::~QOutputSelect()
{
  delete ui;
}

//------------------------------------------------------------------------------
/*! \brief Updates the filter.
 *
 */
void QOutputSelect::update( tvector<tfloat> f )
{
  H = tvector<tcomplex>( length(f) );

  if( fileName.isEmpty() )
  {
    for( tuint ii = 0; ii < length(f); ii++ )
      H[ii] = 1.0;
  }
  else
  {
    tvector<tfloat> magt = abs( FR );
    tvector<tfloat> phit = angle( FR );

    tvector<tfloat> mag = interp1( freq, magt, f, "spline" );
    tvector<tfloat> phi = interp1( freq, phit, f, "spline" );

    H = mag * exp( j*phi );
  }
}

//------------------------------------------------------------------------------
/*!
 *
 */
void QOutputSelect::sendDspParameter( void )
{
  //uint32_t val = static_cast<uint32_t>(ui->comboBoxInput->currentIndex());
  //dsp->sendParameter( addr[kInput], val );
}

//------------------------------------------------------------------------------
/*!
 *
 */
uint32_t QOutputSelect::getNumBytes( void )
{
  return 0;
}

//------------------------------------------------------------------------------
/*!
 *
 */
void QOutputSelect::writeDspParameter( void )
{
  //uint32_t val = static_cast<uint32_t>(ui->comboBoxInput->currentIndex());
  //dsp->storeRegAddr( addr[kOutput] );
  //dsp->storeValue( val );
}

//------------------------------------------------------------------------------
/*!
 *
 */
/*void QOutputSelect::on_comboBoxInput_currentIndexChanged( int  )
{
  sendDspParameter();
  emit valueChanged();
}*/

//------------------------------------------------------------------------------
/*!
 *
 */
bool QOutputSelect::eventFilter( QObject* object, QEvent* event )
{
  if( object == ui->lineEditResponseFile && event->type() == QEvent::MouseButtonDblClick )
  {
    fileName = QFileDialog::getOpenFileName( this, tr("Open Frequency Response"), 
                                             QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory),
                                             tr("FRD Files (*.frd, *.txt)") );

    if( !fileName.isEmpty() )
    {
      QFile fileFRD( fileName );
      QFileInfo infoFRD( fileFRD );

      if( !fileFRD.exists() )
      {
        QMessageBox::critical( this, tr("Loading file failed"),
                                     tr("You did not select a valid frequency response file. File will be ignored. Sorry."),
                                     QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton );
        fileName = "";
      }
      else 
      {
        bool ok;
        double ref = QInputDialog::getDouble( this, tr("Import measurement"),
                                              tr("SPL Reference:"), refSpl, 1.0, 120.0, 1, &ok );
        if( ok )
          refSpl = ref;

        unsigned int idx = 0;
        if( fileFRD.open( QIODevice::ReadOnly ) ) 
        {
          QTextStream in( &fileFRD );
          while( !in.atEnd() ) 
          {
            QString line = in.readLine();
            line = line.simplified();
            QStringList values = line.split( ' ' );
            bool flagNumber;
            tfloat num = values.value(0).toDouble( &flagNumber );
            if( flagNumber ) 
              idx++;
          }
          fileFRD.close();
        }

        freq = tvector<tfloat>( idx );
        FR = tvector<tcomplex>( idx );
        idx = 0;
        if( fileFRD.open( QIODevice::ReadOnly ) ) 
        {
          QTextStream in( &fileFRD );
          while( !in.atEnd() ) 
          {
            QString line = in.readLine();
            line = line.simplified();
            QStringList values = line.split( ' ' );
            bool flagNumber;
            tfloat frq = values.value(0).toDouble( &flagNumber );
            tfloat mag = values.value(1).toDouble();
            tfloat phi = values.value(2).toDouble();
            if( flagNumber ) 
            {
              freq[idx] = frq;
              FR[idx] = std::pow( 10.0, mag/20.0 ) * std::exp( j*phi*pi/180.0 ) / std::pow( 10.0, refSpl/20.0 );
              idx++;
            }
          }
          fileFRD.close();
        }
        ui->lineEditResponseFile->blockSignals( true );
        QFileInfo fileInfo( fileFRD.fileName() );
        ui->lineEditResponseFile->setText( fileInfo.fileName() );
        ui->lineEditResponseFile->blockSignals( false );
      }      
    }
    emit valueChanged();
    return true;
  }
  return false;
}

//==============================================================================
/*!
 */
void QOutputSelect::getUserParams( QByteArray* userParams )
{

}

//==============================================================================
/*!
 */
void QOutputSelect::setUserParams( QByteArray& userParams, int& idx )
{


}