#ifndef _DICOMSENDER_
#define _DICOMSENDER_

#include <string>
#include <boost/thread/mutex.hpp>
#include <boost/filesystem.hpp>
#include "sqlite3.h"

// work around the fact that dcmtk doesn't work in unicode mode, so all string operation needs to be converted from/to mbcs
#ifdef _UNICODE
#undef _UNICODE
#undef UNICODE
#define _UNDEFINEDUNICODE
#endif

#include "dcmtk/ofstd/ofstd.h"
#include "dcmtk/oflog/oflog.h"
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmnet/dimse.h"
#include "dcmtk/dcmnet/diutil.h"

#ifdef _UNDEFINEDUNICODE
#define _UNICODE 1
#define UNICODE 1
#endif

class DICOMSender
{
public:
	DICOMSender(void);
	~DICOMSender(void);

	void Clear();
	
	void DoSend();

	static void DoSendThread(void *obj);

	void Cancel();
	bool IsDone();

	sqlite3 *db;
	std::stringstream mycout;	
	boost::mutex mycoutmutex;
	
	std::string m_PatientName;
	std::string m_NewPatientName;
	std::string m_NewPatientID;
	std::string m_NewBirthDay;

    std::string m_destinationHost;
    unsigned int m_destinationPort;
    std::string m_destinationAETitle;
    std::string m_ourAETitle;

private:
	int Send();
	
	bool IsCanceled();
	void SetDone(bool state);

	boost::mutex mutex;
	bool cancelEvent, doneEvent;

	void WriteLog(const char *msg);
	void WriteLog(const OFCondition &cond);

	OFCondition storeSCU(T_ASC_Association * assoc, const char *fname);
	OFCondition cstore(T_ASC_Association * assoc, const OFString& fname);
	void replacePatientInfoInformation(DcmDataset* dataset);
	// void replaceSOPInstanceInformation(DcmDataset* dataset);
	OFCondition addStoragePresentationContexts(T_ASC_Parameters *params, OFList<OFString>& sopClasses);	
	OFString makeUID(OFString basePrefix, int counter);
	bool updateStringAttributeValue(DcmItem* dataset, const DcmTagKey& key, OFString value);

/*  glue functions to transfer our db to the list*/
	void addfiles();
	static int addimage(void *param,int columns,char** values, char**names);

	OFList<OFString> fileNameList;
	OFList<OFString> sopClassUIDList;    // the list of sop classes

	bool opt_verbose;
    T_ASC_Network *net;
    T_ASC_Parameters *params;
    DIC_NODENAME localHost;
    DIC_NODENAME peerHost;
    T_ASC_Association *assoc;    

	bool opt_showPresentationContexts;
	bool opt_debug;
	bool opt_abortAssociation;
	OFCmdUnsignedInt opt_maxReceivePDULength;
	OFCmdUnsignedInt opt_maxSendPDULength;
	E_TransferSyntax opt_networkTransferSyntax;

	bool unsuccessfulStoreEncountered;
	int lastStatusCode;

	bool opt_proposeOnlyRequiredPresentationContexts;
	bool opt_combineProposedTransferSyntaxes;

	bool opt_correctUIDPadding;
	OFString patientNamePrefix;   // PatientName is PN (maximum 16 chars)
	OFString patientIDPrefix; // PatientID is LO (maximum 64 chars)
	OFString studyIDPrefix;   // StudyID is SH (maximum 16 chars)
	OFString accessionNumberPrefix;  // AccessionNumber is SH (maximum 16 chars)
	bool opt_secureConnection; /* default: no secure connection */		
	T_DIMSE_BlockingMode opt_blockMode;
	int opt_dimse_timeout;
	int opt_acse_timeout;
	int opt_timeout;
	
	OFCmdUnsignedInt opt_compressionLevel;	

	// this is only needed because progress needs it
	T_ASC_PresentationContextID presId;

	static void progressCallback(void * callbackData, T_DIMSE_StoreProgress *progress, T_DIMSE_C_StoreRQ * req);
};

#endif