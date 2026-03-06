# Prerequisites
- **C++**: Familiarity with modern C++
- **OpenGL**: Familiarity with modern OpenGL concepts, including framebuffers, shaders, and rendering pipelines.
- **Pixar’s OpenUSD**: basic knowledge of core USD concepts such as
stages, prims, attributes, and layers.
- **Cmake**: Familiarity with configuring, building, and linking C++ projects
using CMake.
- **Qt**: Comfortable with creating Qt widgets, signal/slot communication
model, and the Qt event loop

# How to Build

## Windows

1. VisualStudio 2022 (Required)
2. CMake (Required)
3. Qt 6 (Required)
4. ninja (optional)

Make sure you are working inside a Developer Command Prompt by launching
VsDevCmd.bat:

```console
cmd.exe /k "C:\\Program Files\\Microsoft Visual
Studio\\2022\\Professional\\Common7\\Tools\\VsDevCmd.bat" -startdir=none
-arch=x64 -host_arch=x64
```

Then run the following commands:

```console
1- git clone https://github.com/hamedsabri/usd-hydra-tutorials && cd usd-hydra-tutorials

2- makedir build && cd build

3- cmake -GNinja -DCMAKE_MAKE_PROGRAM="<path_to_ninja_exe>" -DQT_LOCATION="<path_to_qt_install_directory>" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX="<install_path>" ..

4- ninja -j<num_cores> install
```

Alternatively, you can build using `MSBuild`:

```console
cmake -DQT_LOCATION="<path_to_qt_install_directory>" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX="<install_path>" ..

cmake --build . --config RelWithDebInfo --target install
```

That said, MSBuild is noticeably slower. Switching to Ninja can significantly improve build times.

# Building a Minimal Qt Application With OpenGL Viewport
To start, we need a minimal Qt application that hosts an OpenGL viewport. For the sake of simplicity, only the essential components required for a functional viewport are included.

## MainWindow
The MainWindow initializes the core application components, including the dock manager, menu bar, OpenGL viewport, and logger:

```cpp
auto dockManager = new ads::CDockManager(this);
auto mainMenuBar = new MainMenuBar(this);
auto viewportGLWidget = new ViewportOpenGLWidget(this);
LogWidget& loggerWidget = LogWidget::instance(this);
```

## ViewportOpenGLWidget
The viewport itself is implemented as a QOpenGLWidget and embedded in the main window as a dock widget using the Qt Advanced Docking System (ADS).

```cpp
class ViewportOpenGLWidget
    : public QOpenGLWidget
    , public QOpenGLFunctions_4_5_Core
{
    Q_OBJECT

public:
    ViewportOpenGLWidget(QWidget* parent = nullptr);
    virtual ~ViewportOpenGLWidget() = default;

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
};
```
QOpenGLWidget is Qt's built-in widget designed specifically for OpenGL rendering and provides:

- An OpenGL context
- A default framebuffer object
- Automatic context management
- The render lifecycle callbacks:
- initializeGL()
- resizeGL()
- paintGL()

```cpp
ViewportOpenGLWidget::ViewportOpenGLWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
    QSurfaceFormat format;
    format.setSamples(SAMPLE_AMOUNT);
    setFormat(format);
}

void ViewportOpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
}

void ViewportOpenGLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void ViewportOpenGLWidget::paintGL()
{
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
}

void ViewportOpenGLWidget::wheelEvent(QWheelEvent* event)
{
    update();
}

void ViewportOpenGLWidget::mousePressEvent(QMouseEvent* event)
{
}

void ViewportOpenGLWidget::mouseMoveEvent(QMouseEvent* event)
{
    update();
}

void ViewportOpenGLWidget::mouseReleaseEvent(QMouseEvent* event)
{
}
```

```cpp
// openGlViewport dockWidget
ads::CDockWidget* openGlViewportDockWidget = new ads::CDockWidget("Viewport");
openGlViewportDockWidget->setWidget(viewportGLWidget);

openGlViewportDockWidget->setMinimumSizeHintMode(ads::CDockWidget::MinimumSizeHintFromDockWidget);
openGlViewportDockWidget->setMinimumSize(600, 450);

openGlViewportDockWidget->setFeature(ads::CDockWidget::DockWidgetClosable, false);
openGlViewportDockWidget->setFeature(ads::CDockWidget::DockWidgetMovable, false);
openGlViewportDockWidget->setFeature(ads::CDockWidget::DockWidgetFloatable, false);

dockManager->addDockWidget(ads::CenterDockWidgetArea, openGlViewportDockWidget);
mainMenuBar->getPanelsMenu()->addAction(openGlViewportDockWidget->toggleViewAction());
```
## LogWidget
To make debugging easier, the viewer includes a simple Logger widget. Instead of relying solely on 
console output, this widget captures Qt log messages and displays them directly inside the application,
making it easier to inspect runtime behavior while interacting with the UI.

```cpp
// logger
ads::CDockWidget* loggerDockWidget = new ads::CDockWidget("Logger");
loggerDockWidget->setWidget(&loggerWidget);
loggerDockWidget->setMinimumSizeHintMode(ads::CDockWidget::MinimumSizeHintFromDockWidget);
loggerDockWidget->setMinimumSize(340, 50);
dockManager->addDockWidget(ads::BottomDockWidgetArea, loggerDockWidget);
mainMenuBar->getPanelsMenu()->addAction(loggerDockWidget->toggleViewAction());
```
## MainMenuBar
The menu bar provides the primary user actions such as creating a “new stage” or “opening” an existing one.
```cpp
QAction* newStageAction = new QAction("New Stage", this);
QAction* openStageAction = new QAction("Open Stage", this);
QAction* quitAction = new QAction("Quit", this);
```
# Executable Demo

<img width="1186" height="1002" alt="image" src="https://github.com/user-attachments/assets/00580392-8d32-4f8f-8a72-5dc008cfc479" />

