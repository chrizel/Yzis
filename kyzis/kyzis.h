#ifndef KYZIS_H
#define KYZIS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapplication.h>
#include <kmdimainfrm.h>
#include <dcopobject.h>

/**
 * This is the application "Shell".  It has a menubar, toolbar, and
 * statusbar but relies on the "Part" to do all the real work.
 *
 * @short Application Shell
 * @author Yzis Team <yzis-dev@yzis.org>
 */
class Kyzis : public KMdiMainFrm, public DCOPObject
{
	K_DCOP
    Q_OBJECT
public:
    /**
	 * Constructs a Kyzis widget
	 * @param dockConfig the configuration of dock widgets
	 * @param mode the startup MDI mode
     */
    Kyzis(QDomElement& dockConfig, KMdi::MdiMode mode);

    /**
     * Default Destructor
     */
    virtual ~Kyzis();

    /**
     * Use this method to load whatever file/URL you have
	 * @param url the url to open
     */
    void load(const KURL& url);

k_dcop:
	/**
	 * Opens a new buffer
	 * @param path file to which the buffer is linked
	 */
    void createBuffer(const QString& path);

protected:
	virtual void resizeEvent( QResizeEvent *e );

private slots:
    void fileNew();
    void fileOpen();
    void optionsShowToolbar();
    void optionsConfigureKeys();
    void optionsConfigureToolbars();
    void applyNewToolbarConfig();

private:
    void setupActions();

private:
    KToggleAction *m_toolbarAction;
	QDomElement m_dockConfig;
	KParts::ReadWritePart *m_currentPart;
};

#endif // KYZIS_H
