#include "facerecognitionthread.h"

FaceRecognitionThread::FaceRecognitionThread(const Mat &frame, QObject *parent,MySqlLite *sqltie)
    : QThread(parent), currentFrame(frame.clone())
{
    mySql = sqltie;
}
void FaceRecognitionThread::run() {
//    emit selectFaceFeatureForMSQ();
    recognizeFaceFromMSQ(MYFaceArray);
}
void FaceRecognitionThread::setCurrentFrame(const Mat &frame) {
    currentFrame = frame.clone();
}
void FaceRecognitionThread::setFeatureArray(std::vector<FaceFeature> faceArray) {
    MYFaceArray = faceArray;
}
void FaceRecognitionThread::recognizeFaceFromMSQ(std::vector<FaceFeature> faceArray) {
    int ret = IdFaceSdkInit(".", ".");
    if (ret < 0) {
        qDebug() << "IdFaceSdkInit failed! (ret=" << ret << ")";
        emit errorFaceR("SDK初始化失败，请退出程序");
        emit finished();
        return;
    }

    int nFeatureSize = IdFaceSdkFeatureSize();
    BYTE* pFeature = new BYTE[nFeatureSize];
    BYTE* pFeatureAll = new BYTE[nFeatureSize];

    if (!pFeature || !pFeatureAll) {
        qDebug() << "Failed to allocate feature buffers.";
        if (pFeature) delete[] pFeature;
        if (pFeatureAll) delete[] pFeatureAll;
        IdFaceSdkUninit();
        emit finished();
        return;
    }

    HANDLE hList = IdFaceSdkListCreate(MYFaceArray.size() + 1);
    if (hList == nullptr) {
        qDebug() << "IdFaceSdkListCreate failed.";
        delete[] pFeature;
        delete[] pFeatureAll;
        IdFaceSdkUninit();
        emit finished();
        return;
    }

    int m = 0;
    int item = 1;
    for (const FaceFeature& faceFeature : MYFaceArray) {
        memcpy(pFeatureAll, faceFeature.featureData.constData(), nFeatureSize);
        int nPos = -1;
        m = IdFaceSdkListInsert(hList, &nPos, pFeatureAll, 1);
        if (m != item) {
            qDebug() << "IdFaceSdkListInsert failed for user:" << faceFeature.userNo;
        }
        item++;
    }

    if (m != MYFaceArray.size()) {
        qDebug() << "IdFaceSdkListInsert failed.";
        cleanup(pFeature, pFeatureAll, hList);
        IdFaceSdkUninit();
        emit errorFaceR("没有找到匹配的用户");
        emit finished();
        return;
    }

    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < 10000) {
        if (currentFrame.empty()) {
            qDebug() << "No image data provided.";
            cleanup(pFeature, pFeatureAll, hList);
            emit finished();
            return;
        }

        cv::Mat sampleGray;
        cv::cvtColor(currentFrame, sampleGray, cv::COLOR_BGR2GRAY);

        FACE_DETECT_RESULT face;
        ret = IdFaceSdkDetectFace(sampleGray.data, sampleGray.cols, sampleGray.rows, &face);

        if (ret <= 0) {
//            qDebug() << "IdFaceSdkDetectFace failed. (ret=" << ret << ")";
            emit requestNewFrame();
            continue;
        }

        ret = IdFaceSdkFeatureGet(sampleGray.data, sampleGray.cols, sampleGray.rows, &face, pFeature);
        if (ret != 0) {
            qDebug() << "IdFaceSdkFeatureGet failed. (ret=" << ret << ")";
            emit requestNewFrame();
            continue;
        }

        BYTE* pScores = new BYTE[MYFaceArray.size() + 1];
        int j = IdFaceSdkListCompare(hList, pFeature, 0, -1, pScores);

        if (j != MYFaceArray.size()) {
            cleanup(pFeature, pFeatureAll, hList);
            delete[] pScores;
            IdFaceSdkUninit();
            emit errorFaceR("没有找到匹配的用户");
            emit finished();
            return;
        }

        BYTE nMaxScore = 0;
        int nMatchIndex = -1;

        for (int i = 0; i < MYFaceArray.size(); i++) {
            qDebug() << "Score for user" << i << ":" << static_cast<unsigned char>(pScores[i]);
            if (pScores[i] > nMaxScore) {
                nMaxScore = pScores[i];
                nMatchIndex = i;
            }
        }


        delete[] pScores;


        if (nMatchIndex != -1) {
            qDebug() << "匹配用户成功 for user:" << MYFaceArray[nMatchIndex].userNo;
//            emit errorFaceR("匹配用户成功");


            emit recognitionResult(nMatchIndex, nMaxScore,MYFaceArray[nMatchIndex].userNo);
            cleanup(pFeature, pFeatureAll, hList);
            IdFaceSdkUninit();
            emit finished();
            return;
        }
    }

    // Handle timeout
    cleanup(pFeature, pFeatureAll, hList);
    IdFaceSdkUninit();
    emit errorFaceR("人脸检测失败，超过最大重试次数");
    emit finished();
}

void FaceRecognitionThread::cleanup(BYTE* pFeature, BYTE* pFeatureAll, HANDLE hList) {
    if (pFeature) delete[] pFeature;
    if (pFeatureAll) delete[] pFeatureAll;
    MYFaceArray.clear();
    IdFaceSdkListDestroy(hList);
}
