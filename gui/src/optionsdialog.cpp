#include "optionsdialog.h"
#include "optionsmodel.h"
#include "monitoreddatamapper.h"
#include "guiutil.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QStackedWidget>

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QRegExpValidator>

/* First (currently only) page of options */
class MainOptionsPage : public QWidget
{
public:
    explicit MainOptionsPage(QWidget *parent=0);

    void setMapper(MonitoredDataMapper *mapper);
private:
    QCheckBox *bitcoin_at_startup;
    QCheckBox *minimize_to_tray;
    QCheckBox *map_port_upnp;
    QCheckBox *minimize_on_close;
    QCheckBox *connect_socks4;
    QLineEdit *proxy_ip;
    QLineEdit *proxy_port;
    QLineEdit *fee_edit;

signals:

public slots:

};

OptionsDialog::OptionsDialog(QWidget *parent):
    QDialog(parent), contents_widget(0), pages_widget(0),
    main_options_page(0), model(0)
{
    contents_widget = new QListWidget();
    contents_widget->setMaximumWidth(128);

    pages_widget = new QStackedWidget();
    pages_widget->setMinimumWidth(300);

    QListWidgetItem *item_main = new QListWidgetItem(tr("Main"));
    contents_widget->addItem(item_main);
    main_options_page = new MainOptionsPage(this);
    pages_widget->addWidget(main_options_page);

    contents_widget->setCurrentRow(0);

    QHBoxLayout *main_layout = new QHBoxLayout();
    main_layout->addWidget(contents_widget);
    main_layout->addWidget(pages_widget, 1);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(main_layout);

    QHBoxLayout *buttons = new QHBoxLayout();
    buttons->addStretch(1);
    QPushButton *ok_button = new QPushButton(tr("OK"));
    buttons->addWidget(ok_button);
    QPushButton *cancel_button = new QPushButton(tr("Cancel"));
    buttons->addWidget(cancel_button);
    apply_button = new QPushButton(tr("Apply"));
    apply_button->setEnabled(false);
    buttons->addWidget(apply_button);

    layout->addLayout(buttons);

    setLayout(layout);
    setWindowTitle(tr("Options"));

    /* Widget-to-option mapper */
    mapper = new MonitoredDataMapper(this);
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    mapper->setOrientation(Qt::Vertical);
    /* enable apply button when data modified */
    connect(mapper, SIGNAL(viewModified()), this, SLOT(enableApply()));
    /* disable apply button when new data loaded */
    connect(mapper, SIGNAL(currentIndexChanged(int)), this, SLOT(disableApply()));

    /* Event bindings */
    connect(ok_button, SIGNAL(clicked()), this, SLOT(okClicked()));
    connect(cancel_button, SIGNAL(clicked()), this, SLOT(cancelClicked()));
    connect(apply_button, SIGNAL(clicked()), this, SLOT(applyClicked()));
}

void OptionsDialog::setModel(OptionsModel *model)
{
    this->model = model;

    mapper->setModel(model);
    main_options_page->setMapper(mapper);

    mapper->toFirst();
}

void OptionsDialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);
    if(current)
    {
        pages_widget->setCurrentIndex(contents_widget->row(current));
    }
}

void OptionsDialog::okClicked()
{
    mapper->submit();
    accept();
}

void OptionsDialog::cancelClicked()
{
    reject();
}

void OptionsDialog::applyClicked()
{
    mapper->submit();
    apply_button->setEnabled(false);
}

void OptionsDialog::enableApply()
{
    apply_button->setEnabled(true);
}

void OptionsDialog::disableApply()
{
    apply_button->setEnabled(false);
}

MainOptionsPage::MainOptionsPage(QWidget *parent):
        QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout();

    bitcoin_at_startup = new QCheckBox(tr("&Start Bitcoin on window system startup"));
    bitcoin_at_startup->setToolTip(tr("Automatically start Bitcoin after the computer is turned on"));
    layout->addWidget(bitcoin_at_startup);

    minimize_to_tray = new QCheckBox(tr("&Minimize to the tray instead of the taskbar"));
    minimize_to_tray->setToolTip(tr("Show only a tray icon after minimizing the window"));
    layout->addWidget(minimize_to_tray);

    map_port_upnp = new QCheckBox(tr("Map port using &UPnP"));
    map_port_upnp->setToolTip(tr("Automatically open the Bitcoin client port on the router. This only works when your router supports UPnP and it is enabled."));
    layout->addWidget(map_port_upnp);

    minimize_on_close = new QCheckBox(tr("M&inimize on close"));
    minimize_on_close->setToolTip(tr("Minimize instead of exit the application when the window is closed. When this option is enabled, the application will be closed only after selecting Quit in the menu."));
    layout->addWidget(minimize_on_close);

    connect_socks4 = new QCheckBox(tr("&Connect through SOCKS4 proxy:"));
    connect_socks4->setToolTip(tr("Connect to the Bitcon network through a SOCKS4 proxy (e.g. when connecting through Tor)"));
    layout->addWidget(connect_socks4);

    QHBoxLayout *proxy_hbox = new QHBoxLayout();
    proxy_hbox->addSpacing(18);
    QLabel *proxy_ip_label = new QLabel(tr("Proxy &IP: "));
    proxy_hbox->addWidget(proxy_ip_label);
    proxy_ip = new QLineEdit();
    proxy_ip->setMaximumWidth(140);
    proxy_ip->setEnabled(false);
    proxy_ip->setValidator(new QRegExpValidator(QRegExp("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}"), this));
    proxy_ip->setToolTip(tr("IP address of the proxy (e.g. 127.0.0.1)"));
    proxy_ip_label->setBuddy(proxy_ip);
    proxy_hbox->addWidget(proxy_ip);
    QLabel *proxy_port_label = new QLabel(tr("&Port: "));
    proxy_hbox->addWidget(proxy_port_label);
    proxy_port = new QLineEdit();
    proxy_port->setMaximumWidth(55);
    proxy_port->setValidator(new QIntValidator(0, 65535, this));
    proxy_port->setEnabled(false);
    proxy_port->setToolTip(tr("Port of the proxy (e.g. 1234)"));
    proxy_port_label->setBuddy(proxy_port);
    proxy_hbox->addWidget(proxy_port);
    proxy_hbox->addStretch(1);

    layout->addLayout(proxy_hbox);
    QLabel *fee_help = new QLabel(tr("Optional transaction fee per KB that helps make sure your transactions are processed quickly.  Most transactions are 1KB.  Fee 0.01 recommended."));
    fee_help->setWordWrap(true);
    layout->addWidget(fee_help);

    QHBoxLayout *fee_hbox = new QHBoxLayout();
    fee_hbox->addSpacing(18);
    QLabel *fee_label = new QLabel(tr("Pay transaction &fee"));
    fee_hbox->addWidget(fee_label);
    fee_edit = new QLineEdit();
    fee_edit->setMaximumWidth(100);
    fee_edit->setToolTip(tr("Optional transaction fee per KB that helps make sure your transactions are processed quickly. Most transactions are 1KB. Fee 0.01 recommended."));

    GUIUtil::setupAmountWidget(fee_edit, this);

    fee_label->setBuddy(fee_edit);
    fee_hbox->addWidget(fee_edit);
    fee_hbox->addStretch(1);

    layout->addLayout(fee_hbox);

    layout->addStretch(1); /* Extra space at bottom */

    setLayout(layout);

    connect(connect_socks4, SIGNAL(toggled(bool)), proxy_ip, SLOT(setEnabled(bool)));
    connect(connect_socks4, SIGNAL(toggled(bool)), proxy_port, SLOT(setEnabled(bool)));

#ifndef USE_UPNP
    map_port_upnp->setDisabled(true);
#endif
}

void MainOptionsPage::setMapper(MonitoredDataMapper *mapper)
{
    /* Map model to widgets */
    mapper->addMapping(bitcoin_at_startup, OptionsModel::StartAtStartup);
    mapper->addMapping(minimize_to_tray, OptionsModel::MinimizeToTray);
    mapper->addMapping(map_port_upnp, OptionsModel::MapPortUPnP);
    mapper->addMapping(minimize_on_close, OptionsModel::MinimizeOnClose);
    mapper->addMapping(connect_socks4, OptionsModel::ConnectSOCKS4);
    mapper->addMapping(proxy_ip, OptionsModel::ProxyIP);
    mapper->addMapping(proxy_port, OptionsModel::ProxyPort);
    mapper->addMapping(fee_edit, OptionsModel::Fee);
}

