/****************************************************************************
 * MeshLab                                                           o o     *
 * A versatile mesh processing toolbox                             o     o   *
 *                                                                _   O  _   *
 * Copyright(C) 2005                                                \/)\/    *
 * Visual Computing Lab                                            /\/|      *
 * ISTI - Italian National Research Council                           |      *
 *                                                                    \      *
 * All rights reserved.                                                      *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *   
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
 * for more details.                                                         *
 *                                                                           *
 ****************************************************************************/

#include <math.h>
#include <stdlib.h>
#include <meshlab/glarea.h>
#include <wrap/gl/pick.h>
#include <wrap/qt/gl_label.h>
#include "edit_referencing.h"
#include "edit_referencingDialog.h"

#define MAX_REFPOINTS 128

using namespace std;
using namespace vcg;

EditReferencingPlugin::EditReferencingPlugin() {
	qFont.setFamily("Helvetica");
    qFont.setPixelSize(12);

    referencingDialog = NULL;

    //setup
    usePoint.reserve(MAX_REFPOINTS);
    pointID.reserve(MAX_REFPOINTS);
    pickedPoints.reserve(MAX_REFPOINTS);
    refPoints.reserve(MAX_REFPOINTS);
    pointError.reserve(MAX_REFPOINTS);

    transfMatrix.SetIdentity();

    lastname = 0;
}

const QString EditReferencingPlugin::Info()
{
    return tr("Reference layers using fiducial points.");
}
 
void EditReferencingPlugin::mouseReleaseEvent(QMouseEvent * event, MeshModel &/*m*/, GLArea * gla)
{
    gla->update();
    cur=event->pos();
    //haveToPick=true;

    // DEBUG DEBUG
/*
    vector<vcg::Point3d> FixP;
    vector<vcg::Point3d> MovP;
    Matrix44d Mtrasf;
    vcg::Point3d newp;

    FixP.clear();
    newp = Point3d(-25.19000, 89.52376, 0.0);
    FixP.push_back(newp);
    newp = Point3d(35.97340, 21.47749, 1.0);
    FixP.push_back(newp);
    newp = Point3d(0.00000, 0.00000, 5.0);
    FixP.push_back(newp);
    newp = Point3d(-58.22760, 68.19291, 2.0);
    FixP.push_back(newp);
    newp = Point3d(-24.87419, 29.90908, 1.0);
    FixP.push_back(newp);
    newp = Point3d(8.98064, 46.20274, 2.0);
    FixP.push_back(newp);
    newp = Point3d(-34.08825, 59.46381, -3.0);
    FixP.push_back(newp);
    newp = Point3d(14.83746, 28.60058, -1.0);
    FixP.push_back(newp);

    newp = Point3d(38.97, -45.4012, 0.0);
    MovP.push_back(newp);
    newp = Point3d(-52.9811, -39.7094, 1.0);
    MovP.push_back(newp);
    newp = Point3d(-42.0136, 0.259092, 5.0);
    MovP.push_back(newp);
    newp = Point3d(47.047, -7.04106, 2.0);
    MovP.push_back(newp);
    newp = Point3d(-3.1936, -3.69573, 1.0);
    MovP.push_back(newp);
    newp = Point3d(-16.494, -38.3969, 2.0);
    MovP.push_back(newp);
    newp = Point3d(23.9654, -17.6361, -3.0);
    MovP.push_back(newp);
    newp = Point3d(-32.6295, -29.9274, -1.0);
    MovP.push_back(newp);

    ComputeRigidMatchMatrix(FixP, MovP, Mtrasf);

    this->Log(GLLogStream::FILTER, "MATRIX:");
    this->Log(GLLogStream::FILTER, "%f, %f, %f, %f",Mtrasf[0][0],Mtrasf[0][1],Mtrasf[0][2],Mtrasf[0][3]);
    this->Log(GLLogStream::FILTER, "%f, %f, %f, %f",Mtrasf[1][0],Mtrasf[1][1],Mtrasf[1][2],Mtrasf[1][3]);
    this->Log(GLLogStream::FILTER, "%f, %f, %f, %f",Mtrasf[2][0],Mtrasf[2][1],Mtrasf[2][2],Mtrasf[2][3]);
    this->Log(GLLogStream::FILTER, "%f, %f, %f, %f",Mtrasf[3][0],Mtrasf[3][1],Mtrasf[3][2],Mtrasf[3][3]);

    this->Log(GLLogStream::FILTER, "  ");
    this->Log(GLLogStream::FILTER, "errors:");

    float TrError=0;
    for(int Pind=0; Pind<MovP.size(); Pind++)
    {
      TrError = (FixP[Pind] - (Mtrasf * MovP[Pind])).Norm();
      this->Log(GLLogStream::FILTER, "%d: %f",Pind,TrError);
    }
*/


}
  
void EditReferencingPlugin::Decorate(MeshModel &m, GLArea * gla, QPainter *p)
{
    //status
    int pindex;

    pindex = referencingDialog->ui->tableWidget->currentRow();
    if(pindex == -1)
        status_line1.sprintf("Active Point: ----");
    else
        status_line1.sprintf("Active Point: %s",pointID[pindex].toStdString().c_str());

    this->RealTimeLog("Edit Referencing", m.shortName(),
                      "%s<br>"
                      "%s<br>"
                      "%s<br>"
                      "%s",
                      status_line1.toStdString().c_str(),
                      status_line2.toStdString().c_str(),
                      status_line3.toStdString().c_str(),
                      status_error.toStdString().c_str());

    // draw picked & reference points
    if(true)
    {
        int pindex;
        vcg::Point3d currpoint;
        QString buf;

        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LINE_BIT | GL_DEPTH_BUFFER_BIT);
        glLineWidth(2.0f);

        glDisable(GL_LIGHTING);

        for(pindex=0; pindex<usePoint.size(); pindex++)
        {
            if(usePoint[pindex])    //if active
                glColor3ub(233, 233, 155);
            else
                glColor3ub(115, 115, 75);

            currpoint = pickedPoints[pindex];
            glBegin(GL_LINES);
                glVertex3f(currpoint[0]-10.0, currpoint[1],      currpoint[2]);
                glVertex3f(currpoint[0]+10.0, currpoint[1],      currpoint[2]);
                glVertex3f(currpoint[0],      currpoint[1]-10.0, currpoint[2]);
                glVertex3f(currpoint[0],      currpoint[1]+10.0, currpoint[2]);
                glVertex3f(currpoint[0],      currpoint[1],      currpoint[2]-10.0);
                glVertex3f(currpoint[0],      currpoint[1],      currpoint[2]+10.0);
            glEnd();

            buf = pointID[pindex] + " (picked)";
            vcg::glLabel::render(p,currpoint,buf);

            if(usePoint[pindex])    //if active
                glColor3ub(155, 233, 233);
            else
                glColor3ub(75, 115, 115);

            currpoint = refPoints[pindex];
            glBegin(GL_LINES);
                glVertex3f(currpoint[0]-10.0, currpoint[1],      currpoint[2]);
                glVertex3f(currpoint[0]+10.0, currpoint[1],      currpoint[2]);
                glVertex3f(currpoint[0],      currpoint[1]-10.0, currpoint[2]);
                glVertex3f(currpoint[0],      currpoint[1]+10.0, currpoint[2]);
                glVertex3f(currpoint[0],      currpoint[1],      currpoint[2]-10.0);
                glVertex3f(currpoint[0],      currpoint[1],      currpoint[2]+10.0);
            glEnd();

            buf = pointID[pindex] + " (ref)";
            vcg::glLabel::render(p,currpoint,buf);

            if(validMatrix)
            {
                if(usePoint[pindex])    //if active
                {
                    glColor3ub(128, 255, 128);

                    currpoint = transfMatrix * pickedPoints[pindex];
                    glBegin(GL_LINES);
                        glVertex3f(currpoint[0]-10.0, currpoint[1],      currpoint[2]);
                        glVertex3f(currpoint[0]+10.0, currpoint[1],      currpoint[2]);
                        glVertex3f(currpoint[0],      currpoint[1]-10.0, currpoint[2]);
                        glVertex3f(currpoint[0],      currpoint[1]+10.0, currpoint[2]);
                        glVertex3f(currpoint[0],      currpoint[1],      currpoint[2]-10.0);
                        glVertex3f(currpoint[0],      currpoint[1],      currpoint[2]+10.0);
                    glEnd();

                    buf = pointID[pindex] + " (trasf)";
                    vcg::glLabel::render(p,currpoint,buf);
                }
            }
        }

        glEnable(GL_LIGHTING);
        glPopAttrib();
    }

 if(curFacePtr)
 {
    glPushMatrix();
    glMultMatrix(m.cm.Tr);
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LINE_BIT | GL_DEPTH_BUFFER_BIT);
    glLineWidth(2.0f);
    glDepthFunc(GL_ALWAYS);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
	
    glDisable(GL_LIGHTING);
    glColor(Color4b::DarkRed);
    glBegin(GL_LINE_LOOP);
        glVertex(curFacePtr->P(0));
        glVertex(curFacePtr->P(1));
        glVertex(curFacePtr->P(2));
    glEnd();

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glColor(Color4b::Red);
    glBegin(GL_LINE_LOOP);
        glVertex(curFacePtr->P(0));
        glVertex(curFacePtr->P(1));
        glVertex(curFacePtr->P(2));
    glEnd();

    drawFace(curFacePtr,m,gla,p);

    glPopAttrib();
    glPopMatrix();
 }
}

void EditReferencingPlugin::drawFace(CMeshO::FacePointer fp, MeshModel &m, GLArea *gla, QPainter *p)
{
  QString buf = QString("f%1\n (%3 %4 %5)").arg(tri::Index(m.cm,fp)).arg(tri::Index(m.cm,fp->V(0))).arg(tri::Index(m.cm,fp->V(1))).arg(tri::Index(m.cm,fp->V(2)));
  Point3f c=Barycenter(*fp);
  vcg::glLabel::render(p,c,buf);
  for(int i=0;i<3;++i)
    {
       buf =QString("\nv%1:%2 (%3 %4 %5)").arg(i).arg(fp->V(i) - &m.cm.vert[0]).arg(fp->P(i)[0]).arg(fp->P(i)[1]).arg(fp->P(i)[2]);
      if( m.hasDataMask(MeshModel::MM_VERTQUALITY) )
        buf +=QString(" - Q(%1)").arg(fp->V(i)->Q());
      if( m.hasDataMask(MeshModel::MM_WEDGTEXCOORD) )
          buf +=QString("- uv(%1 %2) id:%3").arg(fp->WT(i).U()).arg(fp->WT(i).V()).arg(fp->WT(i).N());
      if( m.hasDataMask(MeshModel::MM_VERTTEXCOORD) )
          buf +=QString("- uv(%1 %2) id:%3").arg(fp->V(i)->T().U()).arg(fp->V(i)->T().V()).arg(fp->V(i)->T().N());
    vcg::glLabel::render(p,fp->V(i)->P(),buf);
    }
}

bool EditReferencingPlugin::StartEdit(MeshModel &m, GLArea *gla)
{
    qDebug("EDIT_REFERENCING: StartEdit: setup all");
    curFacePtr=0;

    glArea=gla;

    if(referencingDialog == NULL)
    {
        referencingDialog = new edit_referencingDialog(gla->window(), this);

        //connecting buttons
        connect(referencingDialog->ui->addLine,SIGNAL(clicked()),this,SLOT(addNewPoint()));
        connect(referencingDialog->ui->delLine,SIGNAL(clicked()),this,SLOT(deleteCurrentPoint()));
        connect(referencingDialog->ui->pickCurrent,SIGNAL(clicked()),this,SLOT(pickCurrentPoint()));
        connect(referencingDialog->ui->buttonPickRef,SIGNAL(clicked()),this,SLOT(pickCurrentRefPoint()));
        connect(referencingDialog->ui->buttonCalculate,SIGNAL(clicked()),this,SLOT(calculateMatrix()));
        connect(referencingDialog->ui->buttonApply,SIGNAL(clicked()),this,SLOT(applyMatrix()));
        //connecting other actions
    }
    referencingDialog->show();

    // signals for asking clicked point
    connect(gla,SIGNAL(transmitSurfacePos(QString,vcg::Point3f)),this,SLOT(receivedSurfacePoint(QString,vcg::Point3f)));
    connect(this,SIGNAL(askSurfacePos(QString)),gla,SLOT(sendSurfacePos(QString)));

    status_line1 = "";
    status_line2 = "";
    status_line3 = "";
    status_error = "";

    glArea->update();

    return true;
}

void EditReferencingPlugin::EndEdit(MeshModel &/*m*/, GLArea *gla)
{
    qDebug("EDIT_REFERENCING: EndEdit: cleaning all");
    assert(referencingDialog);
    delete(referencingDialog);
    referencingDialog = NULL;

    usePoint.clear();
    pointID.clear();
    pickedPoints.clear();
    refPoints.clear();
    pointError.clear();
}

void EditReferencingPlugin::addNewPoint()
{
    // i do not want to have too many refs
    if(usePoint.size() > MAX_REFPOINTS)
    {
        status_error = "Too many points";
        return;
    }

    QString newname = "P" + QString::number(lastname++);

    // I should check the name is really new... hehe :)

    usePoint.push_back(new bool(true));
    pointID.push_back(newname);
    pickedPoints.push_back(Point3d(0.0, 0.0, 0.0));
    refPoints.push_back(Point3d(0.0, 0.0, 0.0));
    pointError.push_back(0.0);

    // update dialog
    referencingDialog->updateTable();
    glArea->update();
}

void EditReferencingPlugin::deleteCurrentPoint()
{
    int pindex = referencingDialog->ui->tableWidget->currentRow();

    // if nothing selected, skip
    if(pindex == -1)
    {
        status_error = "No point selected";
        return;
    }

    usePoint.erase(usePoint.begin() + pindex);
    pointID.erase(pointID.begin() + pindex);
    pickedPoints.erase(pickedPoints.begin() + pindex);
    refPoints.erase(refPoints.begin() + pindex);
    pointError.erase(pointError.begin() + pindex);

    // update dialog
    referencingDialog->updateTable();
    glArea->update();
}

void EditReferencingPlugin::pickCurrentPoint()
{
    int pindex = referencingDialog->ui->tableWidget->currentRow();

    // if nothing selected, skip
    if(pindex == -1)
    {
        status_error = "No point selected";
        return;
    }

    emit askSurfacePos("current");
    status_line2 = "Double-click on model to pick point";
    glArea->update();
}

void EditReferencingPlugin::pickCurrentRefPoint()
{
    int pindex = referencingDialog->ui->tableWidget->currentRow();

    // if nothing selected, skip
    if(pindex == -1)
    {
        status_error = "No point selected";
        return;
    }

    emit askSurfacePos("currentREF");
    status_line2 = "Double-click on model to pick point";
    glArea->update();
}


void EditReferencingPlugin::receivedSurfacePoint(QString name,vcg::Point3f pPoint)
{
    int pindex = referencingDialog->ui->tableWidget->currentRow();

    if(name=="current")
        pickedPoints[pindex] = Point3d(pPoint[0], pPoint[1], pPoint[2]);
    else
        refPoints[pindex] = Point3d(pPoint[0], pPoint[1], pPoint[2]);

    status_line2 = "";

    // update dialog
    referencingDialog->updateTable();
    glArea->update();
}

void EditReferencingPlugin::calculateMatrix()
{
    vector<vcg::Point3d> FixP;
    vector<vcg::Point3d> MovP;
    vector<int> IndexP;

    int pindex = 0;
    float TrError=0;

    // constructing a vector of only ACTIVE points, plus indices and names, just for convenience
    // matrix calculation function uses all points in the vector, while in the filter we keep
    // a larger lint, in order to turn on and off points when we need, which is more flexible

    FixP.clear();
    FixP.reserve(MAX_REFPOINTS);
    MovP.clear();
    MovP.reserve(MAX_REFPOINTS);
    IndexP.clear();
    IndexP.reserve(MAX_REFPOINTS);

    referencingDialog->ui->buttonApply->setEnabled(false);
    validMatrix=false;
    isMatrixRigid=true;
    status_line3 = "NO MATRIX";

    //filling
    for(pindex=0; pindex<usePoint.size(); pindex++)
    {
        if(usePoint[pindex] == true)
        {
            MovP.push_back(pickedPoints[pindex]);
            FixP.push_back(refPoints[pindex]);
            IndexP.push_back(pindex);
        }

        // while iterating, set all errors to zero
        pointError[pindex]=0.0;
    }

    // if less than 4 points, error
    if(MovP.size() < 4)
    {
        // tell the user the problem
        status_error = "There has to be at least 4 points";

        // cleaning up
        FixP.clear();
        MovP.clear();
        IndexP.clear();

        return;
    }

    if(referencingDialog->ui->cbAllowScaling->checkState() == Qt::Checked)
    {
        this->Log(GLLogStream::FILTER, "calculating NON RIGID transformation using %d reference points:", FixP.size());
        ComputeSimilarityMatchMatrix(FixP, MovP, transfMatrix);
        validMatrix=true;
        isMatrixRigid=false;
    }
    else
    {
        this->Log(GLLogStream::FILTER, "calculating RIGID transformation using %d reference points:", FixP.size());
        ComputeRigidMatchMatrix(FixP, MovP, transfMatrix);
        validMatrix=true;
        isMatrixRigid=true;
    }

    referencingDialog->ui->buttonApply->setEnabled(true);

    status_line3.sprintf("RIGID MATRIX:<br>"
                         "%.2f %.2f %.2f %.3f<br>"
                         "%.2f %.2f %.2f %.3f<br>"
                         "%.2f %.2f %.2f %.3f<br>"
                         "%.2f %.2f %.2f %.3f<br>",
                         transfMatrix[0][0],transfMatrix[0][1],transfMatrix[0][2],transfMatrix[0][3],
                         transfMatrix[1][0],transfMatrix[1][1],transfMatrix[1][2],transfMatrix[1][3],
                         transfMatrix[2][0],transfMatrix[2][1],transfMatrix[2][2],transfMatrix[2][3],
                         transfMatrix[3][0],transfMatrix[3][1],transfMatrix[3][2],transfMatrix[3][3]);

    if(isMatrixRigid)
        this->Log(GLLogStream::FILTER, "RIGID MATRIX:");
    else
        this->Log(GLLogStream::FILTER, "NON-RIGID MATRIX:");
    this->Log(GLLogStream::FILTER, "%f %f %f %f",transfMatrix[0][0],transfMatrix[0][1],transfMatrix[0][2],transfMatrix[0][3]);
    this->Log(GLLogStream::FILTER, "%f %f %f %f",transfMatrix[1][0],transfMatrix[1][1],transfMatrix[1][2],transfMatrix[1][3]);
    this->Log(GLLogStream::FILTER, "%f %f %f %f",transfMatrix[2][0],transfMatrix[2][1],transfMatrix[2][2],transfMatrix[2][3]);
    this->Log(GLLogStream::FILTER, "%f %f %f %f",transfMatrix[3][0],transfMatrix[3][1],transfMatrix[3][2],transfMatrix[3][3]);

    this->Log(GLLogStream::FILTER, "  ");
    this->Log(GLLogStream::FILTER, "Residual Errors:");

    for(pindex=0; pindex<MovP.size(); pindex++)
    {
      TrError = (FixP[IndexP[pindex]] - (transfMatrix * MovP[IndexP[pindex]])).Norm();
      pointError[IndexP[pindex]] = TrError;
      this->Log(GLLogStream::FILTER, "%s: %f",pointID[IndexP[pindex]].toStdString().c_str(),TrError);
    }

    // update dialog
    referencingDialog->updateTable();
    glArea->update();

    // cleaning up
    FixP.clear();
    MovP.clear();
    IndexP.clear();
}

void EditReferencingPlugin::applyMatrix()
{
    glArea->mm()->cm.Tr.FromMatrix(transfMatrix);
    glArea->update();
}