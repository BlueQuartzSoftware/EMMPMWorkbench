/* ============================================================================
* Copyright (c) 2010, Michael A. Jackson (BlueQuartz Software)
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* Redistributions in binary form must reproduce the above copyright notice, this
* list of conditions and the following disclaimer in the documentation and/or
* other materials provided with the distribution.
*
* Neither the name of Michael A. Jackson nor the names of its contributors may
* be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifndef QIMAGEPROCESSINGINTERFACE_H_
#define QIMAGEPROCESSINGINTERFACE_H_


#include <QtCore/QSettings>
#include <QtCore/QtPlugin>
#include <QPair>
#include <QVector>

/**
 * @class QImageProcessingInterface QImageProcessingInterface.h IPHelper/plugins/QImageProcessingInterface.h
 * @brief This class defines the interface used by plugins that would like to process
 * images and display their output.
 * @author Michael A. Jackson for BlueQuartz Software
 * @date Jul 10, 2010
 * @version 1.0
 *
 * @section intro Introduction
 *  In order to develop a plugin for the IPHelper there are several classes that need to be
 *  implemented by the developer.
 *  @li [Plugin Name]Plugin
 *  @li [Plugin Name]InputUI
 *  @li [Plugin Name]Task
 *
 *  The developer should implement all three classes in their own plugin in order to
 *  have a fully functioning plugin that is able to display an input GUI and process
 *  the input images.
 *
 *  @subsection plugin_interface PluginInterface
 *    The developer should create a C++ class that inherits publicly from QImageProcessingInterface and
 *    QObject.
 *    @code
 *    class MyPlugin : public QObject, public QImageProcessingInterface {
 *    @endcode
 *
 *    The developer will also need to add some macro declarations to their class declaration (usually
 *     in the header file.
 *
 *    @code
 *      Q_OBJECT;
 *      Q_INTERFACES(QImageProcessingInterface )
 *    @endcode
 *    The programmer will also need to add the following macros to their implementation file.
 *    @code
 *    Q_EXPORT_PLUGIN2(MyPlugin, MyPlugin);
 *    @endcode
 *
 *    At this point the developer is ready to implement each of the virtual functions in the
 *    QImageProcessingInterface in order to make their plugin valid
 *
 * @subsection plugin_inputui PluginInputUI
 *   In order for the user to be able to set the proper inputs for thier image
 *   processing code the developer will need to design a QWidget based input UI
 *   using QtDesigner or by hand if they wish. An existing class QImageProcessingInputFrame
 *   already has some existing functionality that the developer may wish to readily inherit
 *   from. If that is the case then the developer can simply inherit from QImageProcessingInputFrame
 *   instead of another Qt class as is the norm for QtDesigner based widgets. See the
 *   Qt documentation if you are unfamiliar with how to use Qt Designer to create a GUI.
 *   The developer can also look at source codes for the CrossCorrelationPlugin, CrossCorrelationInputUI
 *   and CrossCorrelationTask source files.
 *
 * @subsection plugin_task PluginTask
 *  If the developer would like to take advantage of the ProcessQueueController and the
 *  ProcessQueueDialog facilities
 *  that are available then they can create a MyPluginTask class that inherits from
 *  the ProcessQueueTask class. They simply need to implement the <tt>run()</tt> method.
 *
 * @section details Details
 */
class QImageProcessingInterface
{
  public:
    virtual ~QImageProcessingInterface(){};

    typedef QPair<QString, QString>        InputOutputFilePair;
    typedef QList<InputOutputFilePair>     InputOutputFilePairList;

    /**
     * @brief Returns the name of the plugin
     */
    virtual QString getPluginName() = 0;

    /**
     * @brief Returns a pointer to the input QWidget that this plugin uses to gather input
     * from the user
     * @param parent The parent QObject to be assigned to the input widget. This will
     * allow Qt's garbage collection scheme to work properly.
     * @return The Input widget used in the GUI.
     */
    virtual QWidget* getInputWidget(QWidget* parent) = 0;

    /**
     * @brief Start the processing of the inputs. This is typically called from the
     * main application that loaded the plugin
     * @param caller The QObject that is calling this function. It is typically used to
     * hook up signals from a ProcessQueueController object instance to signal the
     * main application when the processing has started and finished.
     * @return error value. zero value means NO ERROR has occured.
     */
    virtual int startProcessing(QObject* caller) = 0;

    /**
     * @brief Returns the path to the inputs image as it resides on disk
     * @return Returns the path to the inputs image as it resides on disk
     */
    virtual QString getInputImage() = 0;

    /**
     * @brief Returns the processed image as it was saved on disk by the processing code.
     * @return Returns the processed image as it was saved on disk by the processing code.
     */
    virtual QString getProcessedImage() = 0;

    virtual InputOutputFilePairList getInputOutputFilePairs() = 0;

    /**
     * @brief Writes the settings in the input gui to the Application's preference file
     * @param prefs A valid QSettings pointer.
     */
    virtual void writeSettings(QSettings* prefs) = 0;

    /**
     * @brief Reads the settings from the Application's preference file and sets
     * the input GUI widgets accordingly.
     * @param prefs
     */
    virtual void readSettings(QSettings* prefs) = 0;
};


Q_DECLARE_INTERFACE(QImageProcessingInterface,
                    "net.BlueQuartz.QImageProcessingInterface/1.0")

#endif /* QIMAGEPROCESSINGINTERFACE_H_ */
