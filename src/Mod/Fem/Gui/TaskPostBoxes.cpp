/***************************************************************************
 *   Copyright (c) 2015 Stefan Tröger <stefantroeger@gmx.net>              *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#include "PreCompiled.h"

#ifndef _PreComp_
# include <Inventor/SoPickedPoint.h>
# include <Inventor/events/SoMouseButtonEvent.h>
# include <Inventor/nodes/SoCoordinate3.h>
# include <Inventor/nodes/SoMarkerSet.h>

# include <sstream>

# include <QApplication>
# include <QMessageBox>
#endif

#include <App/Document.h>
#include <Base/Console.h>
#include <Base/UnitsApi.h>
#include <Gui/Action.h>
#include <Gui/Application.h>
#include <Gui/BitmapFactory.h>
#include <Gui/Command.h>
#include <Gui/Document.h>
#include <Gui/MainWindow.h>
#include <Gui/View3DInventor.h>
#include <Gui/View3DInventorViewer.h>
#include <Gui/Inventor/MarkerBitmaps.h>
#include <Mod/Fem/App/FemPostPipeline.h>

#include "ui_TaskPostClip.h"
#include "ui_TaskPostCut.h"
#include "ui_TaskPostDataAlongLine.h"
#include "ui_TaskPostDataAtPoint.h"
#include "ui_TaskPostDisplay.h"
#include "ui_TaskPostScalarClip.h"
#include "ui_TaskPostWarpVector.h"

#include "FemSettings.h"
#include "TaskPostBoxes.h"
#include "ViewProviderFemPostFilter.h"
#include "ViewProviderFemPostFunction.h"
#include "ViewProviderFemPostObject.h"


using namespace FemGui;
using namespace Gui;

// ***************************************************************************

PointMarker::PointMarker(Gui::View3DInventorViewer* iv, std::string ObjName) : view(iv),
    vp(new ViewProviderPointMarker)
{
    view->addViewProvider(vp);
    m_name = ObjName;
}

PointMarker::~PointMarker()
{
    view->removeViewProvider(vp);
    delete vp;
}

void PointMarker::addPoint(const SbVec3f& pt)
{
    int ct = countPoints();
    vp->pCoords->point.set1Value(ct, pt);
}

int PointMarker::countPoints() const
{
    return vp->pCoords->point.getNum();
}

void PointMarker::customEvent(QEvent*)
{
    const SbVec3f& pt1 = vp->pCoords->point[0];
    const SbVec3f& pt2 = vp->pCoords->point[1];

    if (m_name == "DataAlongLine") {
        PointsChanged(pt1[0], pt1[1], pt1[2], pt2[0], pt2[1], pt2[2]);
        Gui::Command::doCommand(Gui::Command::Doc, "App.ActiveDocument.%s.Point1 = App.Vector(%f, %f, %f)", m_name.c_str(), pt1[0], pt1[1], pt1[2]);
        Gui::Command::doCommand(Gui::Command::Doc, "App.ActiveDocument.%s.Point2 = App.Vector(%f, %f, %f)", m_name.c_str(), pt2[0], pt2[1], pt2[2]);
    }
    Gui::Command::doCommand(Gui::Command::Doc, ObjectInvisible().c_str());
}

std::string PointMarker::ObjectInvisible() {
    return "for amesh in App.activeDocument().Objects:\n\
    if \"Mesh\" in amesh.TypeId:\n\
         aparttoshow = amesh.Name.replace(\"_Mesh\",\"\")\n\
         for apart in App.activeDocument().Objects:\n\
             if aparttoshow == apart.Name:\n\
                 apart.ViewObject.Visibility = False\n";
}

PROPERTY_SOURCE(FemGui::ViewProviderPointMarker, Gui::ViewProviderDocumentObject)

ViewProviderPointMarker::ViewProviderPointMarker()
{
    pCoords = new SoCoordinate3();
    pCoords->ref();
    pCoords->point.setNum(0);

    SoGroup* grp = new SoGroup();
    grp->addChild(pCoords);
    addDisplayMaskMode(grp, "Base");
    setDisplayMaskMode("Base");
}

ViewProviderPointMarker::~ViewProviderPointMarker()
{
    pCoords->unref();
}

// ***************************************************************************

DataMarker::DataMarker(Gui::View3DInventorViewer* iv, std::string ObjName) : view(iv),
    vp(new ViewProviderDataMarker)
{
    view->addViewProvider(vp);
    m_name = ObjName;
}

DataMarker::~DataMarker()
{
    view->removeViewProvider(vp);
    delete vp;
}

void DataMarker::addPoint(const SbVec3f& pt)
{
    int ct = countPoints();
    vp->pCoords->point.set1Value(ct, pt);
    vp->pMarker->numPoints = ct + 1;

}

int DataMarker::countPoints() const
{
    return vp->pCoords->point.getNum();
}

void DataMarker::customEvent(QEvent*)
{
    const SbVec3f& pt1 = vp->pCoords->point[0];

    if (m_name == "DataAtPoint") {
        PointsChanged(pt1[0], pt1[1], pt1[2]);
        Gui::Command::doCommand(Gui::Command::Doc, "App.ActiveDocument.%s.Center = App.Vector(%f, %f, %f)", m_name.c_str(), pt1[0], pt1[1], pt1[2]);
    }
    Gui::Command::doCommand(Gui::Command::Doc, ObjectInvisible().c_str());
}

std::string DataMarker::ObjectInvisible() {
    return "for amesh in App.activeDocument().Objects:\n\
    if \"Mesh\" in amesh.TypeId:\n\
         aparttoshow = amesh.Name.replace(\"_Mesh\",\"\")\n\
         for apart in App.activeDocument().Objects:\n\
             if aparttoshow == apart.Name:\n\
                 apart.ViewObject.Visibility = False\n";
}

PROPERTY_SOURCE(FemGui::ViewProviderDataMarker, Gui::ViewProviderDocumentObject)

ViewProviderDataMarker::ViewProviderDataMarker()
{
    pCoords = new SoCoordinate3();
    pCoords->ref();
    pCoords->point.setNum(0);
    pMarker = new SoMarkerSet();
    pMarker->markerIndex = Gui::Inventor::MarkerBitmaps::getMarkerIndex("CIRCLE_FILLED", App::GetApplication().GetParameterGroupByPath("User parameter:BaseApp/Preferences/View")->GetInt("MarkerSize", 9));
    pMarker->numPoints = 0;
    pMarker->ref();

    SoGroup* grp = new SoGroup();
    grp->addChild(pCoords);
    grp->addChild(pMarker);
    addDisplayMaskMode(grp, "Base");
    setDisplayMaskMode("Base");
}

ViewProviderDataMarker::~ViewProviderDataMarker()
{
    pCoords->unref();
    pMarker->unref();
}

// ***************************************************************************
// ***************************************************************************
// TaskDialog
// ***************************************************************************

TaskDlgPost::TaskDlgPost(Gui::ViewProviderDocumentObject* view)
    : TaskDialog(), m_view(view)
{
    assert(view);
}

TaskDlgPost::~TaskDlgPost()
{

}

QDialogButtonBox::StandardButtons TaskDlgPost::getStandardButtons(void) const {

    //check if we only have gui task boxes
    bool guionly = true;
    for (std::vector<TaskPostBox*>::const_iterator it = m_boxes.begin(); it != m_boxes.end(); ++it)
        guionly = guionly && (*it)->isGuiTaskOnly();

    if (!guionly)
        return QDialogButtonBox::Apply | QDialogButtonBox::Ok | QDialogButtonBox::Cancel;
    else
        return QDialogButtonBox::Ok;
}


void TaskDlgPost::appendBox(TaskPostBox* box) {

    m_boxes.push_back(box);
    Content.push_back(box);
}

void TaskDlgPost::open()
{
    // a transaction is already open at creation time of the pad
    QString msg = QObject::tr("Edit post processing object");
    Gui::Command::openCommand((const char*)msg.toUtf8());
}

void TaskDlgPost::clicked(int button)
{
    if (button == QDialogButtonBox::Apply)
        getView()->getObject()->getDocument()->recompute();
}

bool TaskDlgPost::accept()
{

    try {
        std::vector<TaskPostBox*>::iterator it = m_boxes.begin();
        for (; it != m_boxes.end(); ++it)
            (*it)->applyPythonCode();
    }
    catch (const Base::Exception& e) {
        QMessageBox::warning(nullptr, tr("Input error"), QString::fromLatin1(e.what()));
        return false;
    }

    Gui::Command::doCommand(Gui::Command::Gui, "Gui.activeDocument().resetEdit()");
    return true;
}

bool TaskDlgPost::reject()
{
    // roll back the done things
    Gui::Command::abortCommand();
    Gui::Command::doCommand(Gui::Command::Gui, "Gui.activeDocument().resetEdit()");

    return true;
}

void TaskDlgPost::modifyStandardButtons(QDialogButtonBox* box) {

    if (box->button(QDialogButtonBox::Apply))
        box->button(QDialogButtonBox::Apply)->setDefault(true);
}

// ***************************************************************************
// some task box methods
TaskPostBox::TaskPostBox(Gui::ViewProviderDocumentObject* view, const QPixmap& icon, const QString& title, QWidget* parent)
    : TaskBox(icon, title, true, parent) {

    m_view = view;
    m_object = view->getObject();
}

TaskPostBox::~TaskPostBox() {

}

bool TaskPostBox::autoApply() {

    return FemSettings().getPostAutoRecompute();
}

void TaskPostBox::recompute() {

    if (autoApply())
        App::GetApplication().getActiveDocument()->recompute();
}

void TaskPostBox::updateEnumerationList(App::PropertyEnumeration& prop, QComboBox* box) {

    QStringList list;
    std::vector<std::string> vec = prop.getEnumVector();
    for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end(); ++it) {
        list.push_back(QString::fromStdString(*it));
    }

    int index = prop.getValue();
    // be aware the QComboxBox might be connected to the Property,
    // thus clearing the box will set back the property enumeration index too.
    // https://forum.freecadweb.org/viewtopic.php?f=10&t=30944
    box->clear();
    box->insertItems(0, list);
    box->setCurrentIndex(index);
}


// ***************************************************************************
// post pipeline results
TaskPostDisplay::TaskPostDisplay(Gui::ViewProviderDocumentObject* view, QWidget* parent)
    : TaskPostBox(view, Gui::BitmapFactory().pixmap("FEM_ResultShow"), tr("Result display options"), parent)
{
    //we need a separate container widget to add all controls to
    proxy = new QWidget(this);
    ui = new Ui_TaskPostDisplay();
    ui->setupUi(proxy);
    QMetaObject::connectSlotsByName(this);

    this->groupLayout()->addWidget(proxy);

    //update all fields
    updateEnumerationList(getTypedView<ViewProviderFemPostObject>()->DisplayMode, ui->Representation);
    updateEnumerationList(getTypedView<ViewProviderFemPostObject>()->Field, ui->Field);
    updateEnumerationList(getTypedView<ViewProviderFemPostObject>()->VectorMode, ui->VectorMode);

    // get Tranparency from ViewProvider
    int trans = getTypedView<ViewProviderFemPostObject>()->Transparency.getValue();
    Base::Console().Log("Transparency %i: \n", trans);
    // sync the trancparency slider
    ui->Transparency->setValue(trans);
}

TaskPostDisplay::~TaskPostDisplay()
{
    delete ui;
}

void TaskPostDisplay::on_Representation_activated(int i) {

    getTypedView<ViewProviderFemPostObject>()->DisplayMode.setValue(i);
    updateEnumerationList(getTypedView<ViewProviderFemPostObject>()->Field, ui->Field);
    updateEnumerationList(getTypedView<ViewProviderFemPostObject>()->VectorMode, ui->VectorMode);
}

void TaskPostDisplay::on_Field_activated(int i) {

    getTypedView<ViewProviderFemPostObject>()->Field.setValue(i);
    updateEnumerationList(getTypedView<ViewProviderFemPostObject>()->VectorMode, ui->VectorMode);
}

void TaskPostDisplay::on_VectorMode_activated(int i) {

    getTypedView<ViewProviderFemPostObject>()->VectorMode.setValue(i);
}

void TaskPostDisplay::on_Transparency_valueChanged(int i) {

    getTypedView<ViewProviderFemPostObject>()->Transparency.setValue(i);
}

void TaskPostDisplay::applyPythonCode() {

}

// ***************************************************************************
// ?
// the icon fem-post-geo-plane might be wrong but I do not know any better since the plane is one of the implicit functions
TaskPostFunction::TaskPostFunction(ViewProviderDocumentObject* view, QWidget* parent) : TaskPostBox(view, Gui::BitmapFactory().pixmap("fem-post-geo-plane"), tr("Implicit function"), parent) {

    assert(view->isDerivedFrom(ViewProviderFemPostFunction::getClassTypeId()));

    //we load the views widget
    FunctionWidget* w = getTypedView<ViewProviderFemPostFunction>()->createControlWidget();
    w->setParent(this);
    w->setViewProvider(getTypedView<ViewProviderFemPostFunction>());
    this->groupLayout()->addWidget(w);
}

TaskPostFunction::~TaskPostFunction() {

}

void TaskPostFunction::applyPythonCode() {

    //we apply the views widgets python code
}


// ***************************************************************************
// region clip filter
TaskPostClip::TaskPostClip(ViewProviderDocumentObject* view, App::PropertyLink* function, QWidget* parent)
    : TaskPostBox(view, Gui::BitmapFactory().pixmap("FEM_PostFilterClipRegion"), tr("Clip region, choose implicit function"), parent) {

    assert(view->isDerivedFrom(ViewProviderFemPostClip::getClassTypeId()));
    assert(function);
    Q_UNUSED(function);

    fwidget = nullptr;

    //we load the views widget
    proxy = new QWidget(this);
    ui = new Ui_TaskPostClip();
    ui->setupUi(proxy);
    QMetaObject::connectSlotsByName(this);
    this->groupLayout()->addWidget(proxy);

    //the layout for the container widget
    QVBoxLayout* layout = new QVBoxLayout();
    ui->Container->setLayout(layout);

    //fill up the combo box with possible functions
    collectImplicitFunctions();

    //add the function creation command
    Gui::CommandManager& rcCmdMgr = Gui::Application::Instance->commandManager();
    Gui::Command* cmd = rcCmdMgr.getCommandByName("FEM_PostCreateFunctions");
    if (cmd && cmd->getAction())
        cmd->getAction()->addTo(ui->CreateButton);
    ui->CreateButton->setPopupMode(QToolButton::InstantPopup);

    //load the default values
    ui->CutCells->setChecked(static_cast<Fem::FemPostClipFilter*>(getObject())->CutCells.getValue());
    ui->InsideOut->setChecked(static_cast<Fem::FemPostClipFilter*>(getObject())->InsideOut.getValue());
}

TaskPostClip::~TaskPostClip() {

}

void TaskPostClip::applyPythonCode() {

}

void TaskPostClip::collectImplicitFunctions() {

    std::vector<Fem::FemPostPipeline*> pipelines;
    pipelines = App::GetApplication().getActiveDocument()->getObjectsOfType<Fem::FemPostPipeline>();
    if (!pipelines.empty()) {
        Fem::FemPostPipeline* pipeline = pipelines.front();
        if (pipeline->Functions.getValue() &&
            pipeline->Functions.getValue()->getTypeId() == Fem::FemPostFunctionProvider::getClassTypeId()) {

            ui->FunctionBox->clear();
            QStringList items;
            std::size_t currentItem = 0;
            App::DocumentObject* currentFunction = static_cast<Fem::FemPostClipFilter*>(getObject())->Function.getValue();
            const std::vector<App::DocumentObject*>& funcs = static_cast<Fem::FemPostFunctionProvider*>(
                pipeline->Functions.getValue())->Functions.getValues();
            for (std::size_t i = 0; i < funcs.size(); ++i) {
                items.push_back(QString::fromLatin1(funcs[i]->getNameInDocument()));
                if (currentFunction == funcs[i])
                    currentItem = i;
            }
            ui->FunctionBox->addItems(items);
            ui->FunctionBox->setCurrentIndex(currentItem);
        }
    }
}

void TaskPostClip::on_CreateButton_triggered(QAction*) {

    collectImplicitFunctions();
    recompute();
}

void TaskPostClip::on_FunctionBox_currentIndexChanged(int idx) {

    //set the correct property
    std::vector<Fem::FemPostPipeline*> pipelines;
    pipelines = App::GetApplication().getActiveDocument()->getObjectsOfType<Fem::FemPostPipeline>();
    if (!pipelines.empty()) {
        Fem::FemPostPipeline* pipeline = pipelines.front();
        if (pipeline->Functions.getValue() &&
            pipeline->Functions.getValue()->getTypeId() == Fem::FemPostFunctionProvider::getClassTypeId()) {

            const std::vector<App::DocumentObject*>& funcs = static_cast<Fem::FemPostFunctionProvider*>(
                pipeline->Functions.getValue())->Functions.getValues();
            if (idx >= 0)
                static_cast<Fem::FemPostClipFilter*>(getObject())->Function.setValue(funcs[idx]);
            else
                static_cast<Fem::FemPostClipFilter*>(getObject())->Function.setValue(nullptr);
        }
    }

    //load the correct view
    Fem::FemPostFunction* fobj = static_cast<Fem::FemPostFunction*>(
        static_cast<Fem::FemPostClipFilter*>(getObject())->Function.getValue());
    Gui::ViewProvider* view = nullptr;
    if (fobj)
        view = Gui::Application::Instance->activeDocument()->getViewProvider(fobj);

    if (fwidget)
        fwidget->deleteLater();

    if (view) {
        fwidget = static_cast<FemGui::ViewProviderFemPostFunction*>(view)->createControlWidget();
        fwidget->setParent(ui->Container);
        fwidget->setViewProvider(static_cast<FemGui::ViewProviderFemPostFunction*>(view));
        ui->Container->layout()->addWidget(fwidget);
    }
    recompute();
}

void TaskPostClip::on_CutCells_toggled(bool val) {

    static_cast<Fem::FemPostClipFilter*>(getObject())->CutCells.setValue(val);
    recompute();
}

void TaskPostClip::on_InsideOut_toggled(bool val) {

    static_cast<Fem::FemPostClipFilter*>(getObject())->InsideOut.setValue(val);
    recompute();
}


// ***************************************************************************
// data along a line
TaskPostDataAlongLine::TaskPostDataAlongLine(ViewProviderDocumentObject* view, QWidget* parent)
    : TaskPostBox(view, Gui::BitmapFactory().pixmap("FEM_PostFilterDataAlongLine"), tr("Data along a line options"), parent) {

    assert(view->isDerivedFrom(ViewProviderFemPostDataAlongLine::getClassTypeId()));

    //we load the views widget
    proxy = new QWidget(this);
    ui = new Ui_TaskPostDataAlongLine();
    ui->setupUi(proxy);

    QMetaObject::connectSlotsByName(this);
    this->groupLayout()->addWidget(proxy);

    // set decimals before the edits are filled to avoid rounding mistakes
    int UserDecimals = Base::UnitsApi::getDecimals();
    ui->point1X->setDecimals(UserDecimals);
    ui->point1Y->setDecimals(UserDecimals);
    ui->point1Z->setDecimals(UserDecimals);
    ui->point2X->setDecimals(UserDecimals);
    ui->point2Y->setDecimals(UserDecimals);
    ui->point2Z->setDecimals(UserDecimals);

    Base::Unit lengthUnit = static_cast<Fem::FemPostDataAlongLineFilter*>(getObject())->Point1.getUnit();
    ui->point1X->setUnit(lengthUnit);
    ui->point1Y->setUnit(lengthUnit);
    ui->point1Z->setUnit(lengthUnit);
    lengthUnit = static_cast<Fem::FemPostDataAlongLineFilter*>(getObject())->Point2.getUnit();
    ui->point2X->setUnit(lengthUnit);
    ui->point2Y->setUnit(lengthUnit);
    ui->point2Z->setUnit(lengthUnit);

    const Base::Vector3d& vec1 = static_cast<Fem::FemPostDataAlongLineFilter*>(getObject())->Point1.getValue();
    ui->point1X->setValue(vec1.x);
    ui->point1Y->setValue(vec1.y);
    ui->point1Z->setValue(vec1.z);

    const Base::Vector3d& vec2 = static_cast<Fem::FemPostDataAlongLineFilter*>(getObject())->Point2.getValue();
    ui->point2X->setValue(vec2.x);
    ui->point2Y->setValue(vec2.y);
    ui->point2Z->setValue(vec2.z);

    int res = static_cast<Fem::FemPostDataAlongLineFilter*>(getObject())->Resolution.getValue();
    ui->resolution->setValue(res);

    connect(ui->point1X, SIGNAL(valueChanged(double)), this, SLOT(point1Changed(double)));
    connect(ui->point1Y, SIGNAL(valueChanged(double)), this, SLOT(point1Changed(double)));
    connect(ui->point1Z, SIGNAL(valueChanged(double)), this, SLOT(point1Changed(double)));
    connect(ui->point2X, SIGNAL(valueChanged(double)), this, SLOT(point2Changed(double)));
    connect(ui->point2Y, SIGNAL(valueChanged(double)), this, SLOT(point2Changed(double)));
    connect(ui->point2Z, SIGNAL(valueChanged(double)), this, SLOT(point2Changed(double)));
    connect(ui->resolution, SIGNAL(valueChanged(int)), this, SLOT(resolutionChanged(int)));

    //update all fields
    updateEnumerationList(getTypedView<ViewProviderFemPostObject>()->DisplayMode, ui->Representation);
    updateEnumerationList(getTypedView<ViewProviderFemPostObject>()->Field, ui->Field);
    updateEnumerationList(getTypedView<ViewProviderFemPostObject>()->VectorMode, ui->VectorMode);
}

TaskPostDataAlongLine::~TaskPostDataAlongLine() {

}

void TaskPostDataAlongLine::applyPythonCode() {

}

static const char* cursor_triangle[] = {
"32 32 3 1",
"       c None",
".      c #FFFFFF",
"+      c #FF0000",
"      .                         ",
"      .                         ",
"      .                         ",
"      .                         ",
"      .                         ",
"                                ",
".....   .....                   ",
"                                ",
"      .                         ",
"      .                         ",
"      .        ++               ",
"      .       +  +              ",
"      .      + ++ +             ",
"            + ++++ +            ",
"           +  ++ ++ +           ",
"          + ++++++++ +          ",
"         ++  ++  ++  ++         " };

void TaskPostDataAlongLine::on_SelectPoints_clicked() {

    Gui::Command::doCommand(Gui::Command::Doc, ObjectVisible().c_str());
    Gui::Document* doc = Gui::Application::Instance->activeDocument();
    Gui::View3DInventor* view = static_cast<Gui::View3DInventor*>(doc->getActiveView());
    if (view) {
        Gui::View3DInventorViewer* viewer = view->getViewer();
        viewer->setEditing(true);
        viewer->setEditingCursor(QCursor(QPixmap(cursor_triangle), 7, 7));

        // Derives from QObject and we have a parent object, so we don't
        // require a delete.
        std::string ObjName = static_cast<Fem::FemPostDataAlongLineFilter*>(getObject())->Label.getValue();

        FemGui::PointMarker* marker = new FemGui::PointMarker(viewer, ObjName);
        viewer->addEventCallback(SoMouseButtonEvent::getClassTypeId(),
            FemGui::TaskPostDataAlongLine::pointCallback, marker);
        connect(marker, SIGNAL(PointsChanged(double, double, double, double, double, double)), this,
            SLOT(onChange(double, double, double, double, double, double)));
    }
}

std::string TaskPostDataAlongLine::ObjectVisible() {
    return "for amesh in App.activeDocument().Objects:\n\
    if \"Mesh\" in amesh.TypeId:\n\
         aparttoshow = amesh.Name.replace(\"_Mesh\",\"\")\n\
         for apart in App.activeDocument().Objects:\n\
             if aparttoshow == apart.Name:\n\
                 apart.ViewObject.Visibility = True\n";
}

void TaskPostDataAlongLine::on_CreatePlot_clicked() {

    std::string ObjName = static_cast<Fem::FemPostDataAlongLineFilter*>(getObject())->Label.getValue();
    Gui::Command::doCommand(Gui::Command::Doc, "x = App.ActiveDocument.%s.XAxisData", ObjName.c_str());
    Gui::Command::doCommand(Gui::Command::Doc, "y = App.ActiveDocument.%s.YAxisData", ObjName.c_str());
    Gui::Command::doCommand(Gui::Command::Doc, "title = App.ActiveDocument.%s.PlotData", ObjName.c_str());
    Gui::Command::doCommand(Gui::Command::Doc, Plot().c_str());
    recompute();
}

void TaskPostDataAlongLine::onChange(double x1, double y1, double z1, double x2, double y2, double z2) {

    ui->point2X->setValue(x2);
    ui->point2Y->setValue(y2);
    ui->point2Z->setValue(z2);

    ui->point1X->setValue(x1);
    ui->point1Y->setValue(y1);
    ui->point1Z->setValue(z1);
}

void TaskPostDataAlongLine::point1Changed(double) {

    std::string ObjName = static_cast<Fem::FemPostDataAlongLineFilter*>(getObject())->Label.getValue();
    Gui::Command::doCommand(Gui::Command::Doc, "App.ActiveDocument.%s.Point1 = App.Vector(%f, %f, %f)", ObjName.c_str(),
        ui->point1X->value().getValue(), ui->point1Y->value().getValue(), ui->point1Z->value().getValue());
}

void TaskPostDataAlongLine::point2Changed(double) {

    std::string ObjName = static_cast<Fem::FemPostDataAlongLineFilter*>(getObject())->Label.getValue();
    Gui::Command::doCommand(Gui::Command::Doc, "App.ActiveDocument.%s.Point2 = App.Vector(%f, %f, %f)", ObjName.c_str(),
        ui->point2X->value().getValue(), ui->point2Y->value().getValue(), ui->point2Z->value().getValue());
}

void TaskPostDataAlongLine::resolutionChanged(int val) {

    static_cast<Fem::FemPostDataAlongLineFilter*>(getObject())->Resolution.setValue(val);
}

void TaskPostDataAlongLine::pointCallback(void* ud, SoEventCallback* n)
{
    const SoMouseButtonEvent* mbe = static_cast<const SoMouseButtonEvent*>(n->getEvent());
    Gui::View3DInventorViewer* view = reinterpret_cast<Gui::View3DInventorViewer*>(n->getUserData());
    PointMarker* pm = reinterpret_cast<PointMarker*>(ud);

    // Mark all incoming mouse button events as handled, especially, to deactivate the selection node
    n->getAction()->setHandled();

    if (mbe->getButton() == SoMouseButtonEvent::BUTTON1 && mbe->getState() == SoButtonEvent::DOWN) {
        const SoPickedPoint* point = n->getPickedPoint();
        if (point == nullptr) {
            Base::Console().Message("No point picked.\n");
            return;
        }

        n->setHandled();
        pm->addPoint(point->getPoint());
        if (pm->countPoints() == 2) {
            QEvent* e = new QEvent(QEvent::User);
            QApplication::postEvent(pm, e);
            // leave mode
            view->setEditing(false);
            view->removeEventCallback(SoMouseButtonEvent::getClassTypeId(), pointCallback, ud);
        }
    }
    else if (mbe->getButton() != SoMouseButtonEvent::BUTTON1 && mbe->getState() == SoButtonEvent::UP) {
        n->setHandled();
        view->setEditing(false);
        view->removeEventCallback(SoMouseButtonEvent::getClassTypeId(), pointCallback, ud);
        pm->deleteLater();
    }
}

void TaskPostDataAlongLine::on_Representation_activated(int i) {

    getTypedView<ViewProviderFemPostObject>()->DisplayMode.setValue(i);
    updateEnumerationList(getTypedView<ViewProviderFemPostObject>()->Field, ui->Field);
    updateEnumerationList(getTypedView<ViewProviderFemPostObject>()->VectorMode, ui->VectorMode);
}

void TaskPostDataAlongLine::on_Field_activated(int i) {

    getTypedView<ViewProviderFemPostObject>()->Field.setValue(i);
    std::string FieldName = ui->Field->currentText().toStdString();
    static_cast<Fem::FemPostDataAlongLineFilter*>(getObject())->PlotData.setValue(FieldName);
    updateEnumerationList(getTypedView<ViewProviderFemPostObject>()->VectorMode, ui->VectorMode);
}

void TaskPostDataAlongLine::on_VectorMode_activated(int i) {

    getTypedView<ViewProviderFemPostObject>()->VectorMode.setValue(i);
}

std::string TaskPostDataAlongLine::Plot() {
    auto xlabel = tr("Length", "X-Axis plot label");
    std::ostringstream oss;
    oss << "import FreeCAD\n\
from PySide import QtCore\n\
import numpy as np\n\
from matplotlib import pyplot as plt\n\
plt.ioff()\n\
plt.figure(title)\n\
plt.plot(x, y)\n\
plt.xlabel(\"" << xlabel.toStdString() << "\")\n\
plt.ylabel(title)\n\
plt.title(title)\n\
plt.grid()\n\
fig_manager = plt.get_current_fig_manager()\n\
fig_manager.window.setParent(FreeCADGui.getMainWindow())\n\
fig_manager.window.setWindowFlag(QtCore.Qt.Tool)\n\
plt.show()\n";
    return oss.str();
}


// ***************************************************************************
// data at point
TaskPostDataAtPoint::TaskPostDataAtPoint(ViewProviderDocumentObject* view, QWidget* parent)
    : TaskPostBox(view, Gui::BitmapFactory().pixmap("FEM_PostFilterDataAtPoint"), tr("Data at point options"), parent) {

    assert(view->isDerivedFrom(ViewProviderFemPostDataAtPoint::getClassTypeId()));

    //we load the views widget
    proxy = new QWidget(this);
    ui = new Ui_TaskPostDataAtPoint();
    ui->setupUi(proxy);

    QMetaObject::connectSlotsByName(this);
    this->groupLayout()->addWidget(proxy);

    // set decimals before the edits are filled to avoid rounding mistakes
    int UserDecimals = Base::UnitsApi::getDecimals();
    ui->centerX->setDecimals(UserDecimals);
    ui->centerY->setDecimals(UserDecimals);
    ui->centerZ->setDecimals(UserDecimals);

    const Base::Unit lengthUnit = static_cast<Fem::FemPostDataAtPointFilter*>(getObject())->Center.getUnit();
    ui->centerX->setUnit(lengthUnit);
    ui->centerY->setUnit(lengthUnit);
    ui->centerZ->setUnit(lengthUnit);

    const Base::Vector3d& vec = static_cast<Fem::FemPostDataAtPointFilter*>(getObject())->Center.getValue();
    ui->centerX->setValue(vec.x);
    ui->centerY->setValue(vec.y);
    ui->centerZ->setValue(vec.z);

    connect(ui->centerX, SIGNAL(valueChanged(double)), this, SLOT(centerChanged(double)));
    connect(ui->centerY, SIGNAL(valueChanged(double)), this, SLOT(centerChanged(double)));
    connect(ui->centerZ, SIGNAL(valueChanged(double)), this, SLOT(centerChanged(double)));

    //update all fields
    updateEnumerationList(getTypedView<ViewProviderFemPostObject>()->Field, ui->Field);
}

TaskPostDataAtPoint::~TaskPostDataAtPoint() {

}

void TaskPostDataAtPoint::applyPythonCode() {

}

static const char* cursor_star[] = {
"32 17 3 1",
"       c None",
".      c #FFFFFF",
"+      c #FF0000",
"      .                         ",
"      .                         ",
"      .                         ",
"      .                         ",
"      .                         ",
"                                ",
".....   .....                   ",
"                                ",
"      .                         ",
"      .                         ",
"      .        ++               ",
"      .       +  +              ",
"      .      + ++ +             ",
"            + ++++ +            ",
"           +  ++ ++ +           ",
"          + ++++++++ +          ",
"         ++  ++  ++  ++         " };

void TaskPostDataAtPoint::on_SelectPoint_clicked() {

    Gui::Command::doCommand(Gui::Command::Doc, ObjectVisible().c_str());
    Gui::Document* doc = Gui::Application::Instance->activeDocument();
    Gui::View3DInventor* view = static_cast<Gui::View3DInventor*>(doc->getActiveView());
    if (view) {
        Gui::View3DInventorViewer* viewer = view->getViewer();
        viewer->setEditing(true);
        viewer->setEditingCursor(QCursor(QPixmap(cursor_star), 7, 7));

        // Derives from QObject and we have a parent object, so we don't
        // require a delete.
        std::string ObjName = static_cast<Fem::FemPostDataAtPointFilter*>(getObject())->Label.getValue();

        FemGui::DataMarker* marker = new FemGui::DataMarker(viewer, ObjName);
        viewer->addEventCallback(SoMouseButtonEvent::getClassTypeId(),
            FemGui::TaskPostDataAtPoint::pointCallback, marker);
        connect(marker, SIGNAL(PointsChanged(double, double, double)), this, SLOT(onChange(double, double, double)));
    }
    getTypedView<ViewProviderFemPostObject>()->DisplayMode.setValue(1);
    updateEnumerationList(getTypedView<ViewProviderFemPostObject>()->Field, ui->Field);
}

std::string TaskPostDataAtPoint::ObjectVisible() {
    return "for amesh in App.activeDocument().Objects:\n\
    if \"Mesh\" in amesh.TypeId:\n\
         aparttoshow = amesh.Name.replace(\"_Mesh\",\"\")\n\
         for apart in App.activeDocument().Objects:\n\
             if aparttoshow == apart.Name:\n\
                 apart.ViewObject.Visibility = True\n";
}

void TaskPostDataAtPoint::onChange(double x, double y, double z) {

    ui->centerX->setValue(x);
    ui->centerY->setValue(y);
    ui->centerZ->setValue(z);
}

void TaskPostDataAtPoint::centerChanged(double) {

    std::string ObjName = static_cast<Fem::FemPostDataAtPointFilter*>(getObject())->Label.getValue();
    Gui::Command::doCommand(Gui::Command::Doc, "App.ActiveDocument.%s.Center = App.Vector(%f, %f, %f)", ObjName.c_str(),
        ui->centerX->value().getValue(), ui->centerY->value().getValue(), ui->centerZ->value().getValue());
}

void TaskPostDataAtPoint::pointCallback(void* ud, SoEventCallback* n)
{
    const SoMouseButtonEvent* mbe = static_cast<const SoMouseButtonEvent*>(n->getEvent());
    Gui::View3DInventorViewer* view = reinterpret_cast<Gui::View3DInventorViewer*>(n->getUserData());
    DataMarker* pm = reinterpret_cast<DataMarker*>(ud);

    // Mark all incoming mouse button events as handled, especially, to deactivate the selection node
    n->getAction()->setHandled();

    if (mbe->getButton() == SoMouseButtonEvent::BUTTON1 && mbe->getState() == SoButtonEvent::DOWN) {
        const SoPickedPoint* point = n->getPickedPoint();
        if (point == nullptr) {
            Base::Console().Message("No point picked.\n");
            return;
        }

        n->setHandled();
        pm->addPoint(point->getPoint());
        if (pm->countPoints() == 1) {
            QEvent* e = new QEvent(QEvent::User);
            QApplication::postEvent(pm, e);
            // leave mode
            view->setEditing(false);
            view->removeEventCallback(SoMouseButtonEvent::getClassTypeId(), pointCallback, ud);
        }
    }
    else if (mbe->getButton() != SoMouseButtonEvent::BUTTON1 && mbe->getState() == SoButtonEvent::UP) {
        n->setHandled();
        view->setEditing(false);
        view->removeEventCallback(SoMouseButtonEvent::getClassTypeId(), pointCallback, ud);
        pm->deleteLater();
    }
}

void TaskPostDataAtPoint::on_Field_activated(int i) {

    getTypedView<ViewProviderFemPostObject>()->Field.setValue(i);
    std::string FieldName = ui->Field->currentText().toStdString();
    // there is no "None" for the FieldName property, thus return here
    if (FieldName == "None") {
        static_cast<Fem::FemPostDataAtPointFilter*>(getObject())->Unit.setValue("");
        return;
    }
    static_cast<Fem::FemPostDataAtPointFilter*>(getObject())->FieldName.setValue(FieldName);
    if ((FieldName == "von Mises Stress") || (FieldName == "Tresca Stress")
        || (FieldName == "Major Principal Stress") || (FieldName == "Intermediate Principal Stress")
        || (FieldName == "Minor Principal Stress")
        || (FieldName == "Stress xx component") || (FieldName == "Stress xy component")
        || (FieldName == "Stress xz component") || (FieldName == "Stress yy component")
        || (FieldName == "Stress yz component") || (FieldName == "Stress zz component")) {
        static_cast<Fem::FemPostDataAtPointFilter*>(getObject())->Unit.setValue("Pa");
    }
    else if ((FieldName == "Displacement") || (FieldName == "Displacement Magnitude")) {
        static_cast<Fem::FemPostDataAtPointFilter*>(getObject())->Unit.setValue("m");
    }
    else if (FieldName == "Temperature") {
        static_cast<Fem::FemPostDataAtPointFilter*>(getObject())->Unit.setValue("K");
    }
    else if (FieldName == "electric field") {
        static_cast<Fem::FemPostDataAtPointFilter*>(getObject())->Unit.setValue("V/m");
    }
    else if (FieldName == "potential") {
        static_cast<Fem::FemPostDataAtPointFilter*>(getObject())->Unit.setValue("V");
    }
    else if (FieldName == "electric energy density") {
        static_cast<Fem::FemPostDataAtPointFilter*>(getObject())->Unit.setValue("J/m^3");
    }
    // ToDo: set a proper unit once it is known
    else if (FieldName == "potential loads") {
        static_cast<Fem::FemPostDataAtPointFilter*>(getObject())->Unit.setValue("");
    }
    else {
        static_cast<Fem::FemPostDataAtPointFilter*>(getObject())->Unit.setValue("");
    }

    std::string PointData = " The value at that location is "
        + std::to_string(static_cast<Fem::FemPostDataAtPointFilter*>(getObject())->PointData[0])
        + " " + static_cast<Fem::FemPostDataAtPointFilter*>(getObject())->Unit.getValue() + "\n";
    QMessageBox::information(Gui::getMainWindow(),
        qApp->translate("CmdFemPostCreateDataAtPointFilter", "Data At Point"),
        qApp->translate("CmdFemPostCreateDataAtPointFilter", PointData.c_str()));
    Base::Console().Error(PointData.c_str());
}


// ***************************************************************************
// scalar clip filter
TaskPostScalarClip::TaskPostScalarClip(ViewProviderDocumentObject* view, QWidget* parent) :
    TaskPostBox(view, Gui::BitmapFactory().pixmap("FEM_PostFilterClipScalar"), tr("Scalar clip options"), parent) {

    assert(view->isDerivedFrom(ViewProviderFemPostScalarClip::getClassTypeId()));

    //we load the views widget
    proxy = new QWidget(this);
    ui = new Ui_TaskPostScalarClip();
    ui->setupUi(proxy);
    QMetaObject::connectSlotsByName(this);
    this->groupLayout()->addWidget(proxy);

    //load the default values
    updateEnumerationList(getTypedObject<Fem::FemPostScalarClipFilter>()->Scalars, ui->Scalar);
    ui->InsideOut->setChecked(static_cast<Fem::FemPostScalarClipFilter*>(getObject())->InsideOut.getValue());
    App::PropertyFloatConstraint& scalar_prop = static_cast<Fem::FemPostScalarClipFilter*>(getObject())->Value;
    double scalar_factor = scalar_prop.getValue();

    // set spinbox scalar_factor, don't forget to sync the slider
    ui->Value->blockSignals(true);
    ui->Value->setValue(scalar_factor);
    ui->Value->blockSignals(false);

    // sync the slider
    // slider min = 0%, slider max = 100%
    //
    //                 scalar_factor 
    // slider_value = --------------- x 100
    //                      max
    //
    double max = scalar_prop.getConstraints()->UpperBound;
    int slider_value = (scalar_factor / max) * 100.;
    ui->Slider->blockSignals(true);
    ui->Slider->setValue(slider_value);
    ui->Slider->blockSignals(false);
    Base::Console().Log("init: scalar_factor, slider_value: %f, %i: \n", scalar_factor, slider_value);
}

TaskPostScalarClip::~TaskPostScalarClip() {

}

void TaskPostScalarClip::applyPythonCode() {

}

void TaskPostScalarClip::on_Scalar_currentIndexChanged(int idx) {

    static_cast<Fem::FemPostScalarClipFilter*>(getObject())->Scalars.setValue(idx);
    recompute();

    // update constraints and values
    App::PropertyFloatConstraint& scalar_prop = static_cast<Fem::FemPostScalarClipFilter*>(getObject())->Value;
    double scalar_factor = scalar_prop.getValue();
    double min = scalar_prop.getConstraints()->LowerBound;
    double max = scalar_prop.getConstraints()->UpperBound;

    ui->Maximum->setText(QString::number(min));
    ui->Minimum->setText(QString::number(max));

    // set scalar_factor, don't forget to sync the slider
    ui->Value->blockSignals(true);
    ui->Value->setValue(scalar_factor);
    ui->Value->blockSignals(false);

    // sync the slider
    ui->Slider->blockSignals(true);
    int slider_value = (scalar_factor / max) * 100.;
    ui->Slider->setValue(slider_value);
    ui->Slider->blockSignals(false);
}

void TaskPostScalarClip::on_Slider_valueChanged(int v) {

    App::PropertyFloatConstraint& value = static_cast<Fem::FemPostScalarClipFilter*>(getObject())->Value;
    double val = value.getConstraints()->LowerBound * (1 - double(v) / 100.) + double(v) / 100. * value.getConstraints()->UpperBound;

    value.setValue(val);
    recompute();

    //don't forget to sync the spinbox
    ui->Value->blockSignals(true);
    ui->Value->setValue(val);
    ui->Value->blockSignals(false);
}

void TaskPostScalarClip::on_Value_valueChanged(double v) {

    App::PropertyFloatConstraint& value = static_cast<Fem::FemPostScalarClipFilter*>(getObject())->Value;
    value.setValue(v);
    recompute();

    //don't forget to sync the slider
    ui->Slider->blockSignals(true);
    ui->Slider->setValue(int(((v - value.getConstraints()->LowerBound) / (value.getConstraints()->UpperBound - value.getConstraints()->LowerBound)) * 100.));
    ui->Slider->blockSignals(false);
}

void TaskPostScalarClip::on_InsideOut_toggled(bool val) {

    static_cast<Fem::FemPostScalarClipFilter*>(getObject())->InsideOut.setValue(val);
    recompute();
}


// ***************************************************************************
// warp filter
// spinbox min, slider, spinbox max
// spinbox warp factor
TaskPostWarpVector::TaskPostWarpVector(ViewProviderDocumentObject* view, QWidget* parent) :
    TaskPostBox(view, Gui::BitmapFactory().pixmap("FEM_PostFilterWarp"), tr("Warp options"), parent) {

    assert(view->isDerivedFrom(ViewProviderFemPostWarpVector::getClassTypeId()));

    // we load the views widget
    proxy = new QWidget(this);
    ui = new Ui_TaskPostWarpVector();
    ui->setupUi(proxy);
    QMetaObject::connectSlotsByName(this);
    this->groupLayout()->addWidget(proxy);

    // load the default values for warp display
    updateEnumerationList(getTypedObject<Fem::FemPostWarpVectorFilter>()->Vector, ui->Vector);
    double warp_factor = static_cast<Fem::FemPostWarpVectorFilter*>(getObject())->Factor.getValue(); // get the standard warp factor

    // set spinbox warp_factor, don't forget to sync the slider
    ui->Value->blockSignals(true);
    ui->Value->setValue(warp_factor);
    ui->Value->blockSignals(false);

    // set min and max, don't forget to sync the slider
    // TODO if warp is set to standard 1.0, find a smarter way for standard min, max and warp_factor
    // may be depend on grid boundbox and min max vector values
    ui->Max->blockSignals(true);
    ui->Max->setValue(warp_factor == 0 ? 1 : warp_factor * 10.);
    ui->Max->blockSignals(false);
    ui->Min->blockSignals(true);
    ui->Min->setValue(warp_factor == 0 ? 0 : warp_factor / 10.);
    ui->Min->blockSignals(false);

    // sync slider
    ui->Slider->blockSignals(true);
    // slider min = 0%, slider max = 100%
    //
    //                 ( warp_factor - min )
    // slider_value = ----------------------- x 100
    //                     ( max - min )
    //
    int slider_value = (warp_factor - ui->Min->value()) / (ui->Max->value() - ui->Min->value()) * 100.;
    ui->Slider->setValue(slider_value);
    ui->Slider->blockSignals(false);
    Base::Console().Log("init: warp_factor, slider_value: %f, %i: \n", warp_factor, slider_value);
}

TaskPostWarpVector::~TaskPostWarpVector() {

}

void TaskPostWarpVector::applyPythonCode() {

}

void TaskPostWarpVector::on_Vector_currentIndexChanged(int idx) {
    // combobox to choose the result to warp

    static_cast<Fem::FemPostWarpVectorFilter*>(getObject())->Vector.setValue(idx);
    recompute();
}

void TaskPostWarpVector::on_Slider_valueChanged(int slider_value) {
    // slider changed, change warp factor and sync spinbox

    //
    //                                       ( max - min )
    // warp_factor = min + ( slider_value x --------------- )
    //                                            100
    //
    double warp_factor = ui->Min->value() + ((ui->Max->value() - ui->Min->value()) / 100.) * slider_value;
    static_cast<Fem::FemPostWarpVectorFilter*>(getObject())->Factor.setValue(warp_factor);
    recompute();

    // sync the spinbox
    ui->Value->blockSignals(true);
    ui->Value->setValue(warp_factor);
    ui->Value->blockSignals(false);
    Base::Console().Log("Change: warp_factor, slider_value: %f, %i: \n", warp_factor, slider_value);
}

void TaskPostWarpVector::on_Value_valueChanged(double warp_factor) {
    // spinbox changed, change warp factor and sync slider

    // TODO warp factor should not be smaller than min and greater than max, but problems on automate change of warp_factor, see on_Max_valueChanged

    static_cast<Fem::FemPostWarpVectorFilter*>(getObject())->Factor.setValue(warp_factor);
    recompute();

    // sync the slider, see above for formula
    ui->Slider->blockSignals(true);
    int slider_value = (warp_factor - ui->Min->value()) / (ui->Max->value() - ui->Min->value()) * 100.;
    ui->Slider->setValue(slider_value);
    ui->Slider->blockSignals(false);
    Base::Console().Log("Change: warp_factor, slider_value: %f, %i: \n", warp_factor, slider_value);
}

void TaskPostWarpVector::on_Max_valueChanged(double) {

    // TODO max should be greater than min, see a few lines later on problem on input characters
    ui->Slider->blockSignals(true);
    ui->Slider->setValue((ui->Value->value() - ui->Min->value()) / (ui->Max->value() - ui->Min->value()) * 100.);
    ui->Slider->blockSignals(false);

    /*
     * problem, if warp_factor is 2000 one would like to input 4000 as max, one starts to input 4
     * immediately the warp_factor is changed to 4 because 4 < 2000, but one has just input one character of their 4000
     * I do not know how to solve this, but the code to set slider and spinbox is fine thus I leave it ...
     *
     * mhh it works if "apply changes to pipeline directly" button is deactivated, still it really confuses if
     * the button is active. More investigation is needed.
     *
    // set warp factor to max, if warp factor > max
    if (ui->Value->value() > ui->Max->value()) {
        double warp_factor = ui->Max->value();
        static_cast<Fem::FemPostWarpVectorFilter*>(getObject())->Factor.setValue(warp_factor);
        recompute();

        // sync the slider, see above for formula
        ui->Slider->blockSignals(true);
        int slider_value = (warp_factor - ui->Min->value()) / (ui->Max->value() - ui->Min->value()) * 100.;
        ui->Slider->setValue(slider_value);
        ui->Slider->blockSignals(false);
        // sync the spinbox, see above for formula
        ui->Value->blockSignals(true);
        ui->Value->setValue(warp_factor);
        ui->Value->blockSignals(false);
    Base::Console().Log("Change: warp_factor, slider_value: %f, %i: \n", warp_factor, slider_value);
    }
    */
}

void TaskPostWarpVector::on_Min_valueChanged(double) {

    // TODO min should be smaller than max
    // TODO if warp factor is smaller than min, warp factor should be min, don't forget to sync
    ui->Slider->blockSignals(true);
    ui->Slider->setValue((ui->Value->value() - ui->Min->value()) / (ui->Max->value() - ui->Min->value()) * 100.);
    ui->Slider->blockSignals(false);
}


// ***************************************************************************
// function clip filter
TaskPostCut::TaskPostCut(ViewProviderDocumentObject* view, App::PropertyLink* function, QWidget* parent)
    : TaskPostBox(view, Gui::BitmapFactory().pixmap("FEM_PostFilterCutFunction"), tr("Function cut, choose implicit function"), parent) {

    assert(view->isDerivedFrom(ViewProviderFemPostCut::getClassTypeId()));
    assert(function);
    Q_UNUSED(function)

        fwidget = nullptr;

    //we load the views widget
    proxy = new QWidget(this);
    ui = new Ui_TaskPostCut();
    ui->setupUi(proxy);
    QMetaObject::connectSlotsByName(this);
    this->groupLayout()->addWidget(proxy);

    //the layout for the container widget
    QVBoxLayout* layout = new QVBoxLayout();
    ui->Container->setLayout(layout);

    //fill up the combo box with possible functions
    collectImplicitFunctions();

    //add the function creation command
    Gui::CommandManager& rcCmdMgr = Gui::Application::Instance->commandManager();
    Gui::Command* cmd = rcCmdMgr.getCommandByName("FEM_PostCreateFunctions");
    if (cmd && cmd->getAction())
        cmd->getAction()->addTo(ui->CreateButton);
    ui->CreateButton->setPopupMode(QToolButton::InstantPopup);
}

TaskPostCut::~TaskPostCut() {

}

void TaskPostCut::applyPythonCode() {

}

void TaskPostCut::collectImplicitFunctions() {

    std::vector<Fem::FemPostPipeline*> pipelines;
    pipelines = App::GetApplication().getActiveDocument()->getObjectsOfType<Fem::FemPostPipeline>();
    if (!pipelines.empty()) {
        Fem::FemPostPipeline* pipeline = pipelines.front();
        if (pipeline->Functions.getValue() &&
            pipeline->Functions.getValue()->getTypeId() == Fem::FemPostFunctionProvider::getClassTypeId()) {

            ui->FunctionBox->clear();
            QStringList items;
            std::size_t currentItem = 0;
            App::DocumentObject* currentFunction = static_cast<Fem::FemPostClipFilter*>(getObject())->Function.getValue();
            const std::vector<App::DocumentObject*>& funcs = static_cast<Fem::FemPostFunctionProvider*>(
                pipeline->Functions.getValue())->Functions.getValues();
            for (std::size_t i = 0; i < funcs.size(); ++i) {
                items.push_back(QString::fromLatin1(funcs[i]->getNameInDocument()));
                if (currentFunction == funcs[i])
                    currentItem = i;
            }
            ui->FunctionBox->addItems(items);
            ui->FunctionBox->setCurrentIndex(currentItem);
        }
    }
}

void TaskPostCut::on_CreateButton_triggered(QAction*) {

    collectImplicitFunctions();
    recompute();
}

void TaskPostCut::on_FunctionBox_currentIndexChanged(int idx) {

    //set the correct property
    std::vector<Fem::FemPostPipeline*> pipelines;
    pipelines = App::GetApplication().getActiveDocument()->getObjectsOfType<Fem::FemPostPipeline>();
    if (!pipelines.empty()) {
        Fem::FemPostPipeline* pipeline = pipelines.front();
        if (pipeline->Functions.getValue() &&
            pipeline->Functions.getValue()->getTypeId() == Fem::FemPostFunctionProvider::getClassTypeId()) {

            const std::vector<App::DocumentObject*>& funcs = static_cast<Fem::FemPostFunctionProvider*>(
                pipeline->Functions.getValue())->Functions.getValues();
            if (idx >= 0)
                static_cast<Fem::FemPostCutFilter*>(getObject())->Function.setValue(funcs[idx]);
            else
                static_cast<Fem::FemPostCutFilter*>(getObject())->Function.setValue(nullptr);
        }
    }

    //load the correct view
    Fem::FemPostFunction* fobj = static_cast<Fem::FemPostFunction*>(
        static_cast<Fem::FemPostCutFilter*>(getObject())->Function.getValue());
    Gui::ViewProvider* view = nullptr;
    if (fobj)
        view = Gui::Application::Instance->activeDocument()->getViewProvider(fobj);

    if (fwidget)
        fwidget->deleteLater();

    if (view) {
        fwidget = static_cast<FemGui::ViewProviderFemPostFunction*>(view)->createControlWidget();
        fwidget->setParent(ui->Container);
        fwidget->setViewProvider(static_cast<FemGui::ViewProviderFemPostFunction*>(view));
        ui->Container->layout()->addWidget(fwidget);
    }
    recompute();
}

#include "moc_TaskPostBoxes.cpp"
