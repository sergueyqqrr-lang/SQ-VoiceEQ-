# Analog API EQ

Plugin de EQ para voces, formato **VST3 / AU**, construido con **JUCE**.

## Características

- **Bandas ilimitadas en la práctica** (hasta 12 simultáneas activas/inactivas, estilo FabFilter Pro-Q):
  High Pass, Low Shelf, Bell, High Shelf, Low Pass, Notch.
- **Proportional Q estilo API**: al aumentar la ganancia de una banda, su ancho se
  estrecha automáticamente (activable/desactivable por banda), imitando el
  comportamiento de las consolas API 550/560.
- **Saturación analógica** global (drive) con sobremuestreo x2 para dar calidez
  y armónicos sutiles, ideal para voces.
- **Interfaz moderna**: curva de EQ interactiva con nodos arrastrables,
  analizador de espectro en tiempo real de fondo, y panel de control preciso
  por banda.

## Interacción con la curva

- **Arrastrar nodo verticalmente** → ganancia
- **Arrastrar nodo horizontalmente** → frecuencia
- **Rueda del mouse sobre un nodo** → Q
- **Doble click en la curva** → activa una banda nueva ahí mismo
- **Click derecho en un nodo** → cambiar tipo de filtro, alternar Proportional Q, o desactivar la banda

## Requisitos para compilar

- **CMake** 3.22+
- **Windows**: Visual Studio 2022 (con "Desktop development with C++")
- **Mac**: Xcode 14+ y línea de comandos de Xcode (`xcode-select --install`)
- Conexión a internet (CMake descarga JUCE automáticamente la primera vez)

No necesitas instalar JUCE manualmente: `CMakeLists.txt` lo descarga vía
`FetchContent` desde GitHub la primera vez que compiles.

## Compilar en Windows

```bat
cd AnalogEQ
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

El VST3 resultante queda en:
`build\AnalogAPI_EQ_artefacts\Release\VST3\Analog API EQ.vst3`

Cópialo (o el CMake ya lo copia automáticamente, gracias a
`COPY_PLUGIN_AFTER_BUILD TRUE`) a:
`C:\Program Files\Common Files\VST3\`

## Compilar en macOS

```bash
cd AnalogEQ
cmake -B build -G Xcode
cmake --build build --config Release
```

El AU y VST3 se instalan automáticamente (por `COPY_PLUGIN_AFTER_BUILD`) en:
- `~/Library/Audio/Plug-Ins/VST3/`
- `~/Library/Audio/Plug-Ins/Components/` (AU)

Si Studio One no lo detecta, valida el Audio Unit con:
```bash
auval -v aufx Aeq1 Tues
```

## Cargarlo en Studio One

1. Abre Studio One → **Studio One → Options/Preferences → Locations → VST Plug-Ins**
   y confirma que la carpeta de VST3 esté agregada.
2. Ve a **Studio One → Options → Locations → VST Plug-Ins → Reset & Rescan**
   (o simplemente reinicia Studio One).
3. Busca "Analog API EQ" en el navegador de efectos, dentro de la categoría **EQ**.
4. Arrástralo a un canal de voz como inserto.

## Estructura del proyecto

```
AnalogEQ/
├── CMakeLists.txt
└── Source/
    ├── PluginProcessor.h/.cpp     # DSP, parámetros, procesamiento de audio
    ├── PluginEditor.h/.cpp        # Ensamblaje de la interfaz
    ├── DSP/
    │   ├── EQBand.h/.cpp          # Filtro por banda + modelo Proportional Q
    │   └── AnalogSaturator.h      # Saturación analógica con sobremuestreo
    └── GUI/
        ├── LookAndFeel.h/.cpp     # Tema visual oscuro/ámbar
        ├── EQCurveComponent.h/.cpp# Curva interactiva + nodos arrastrables
        ├── SpectrumAnalyzer.h/.cpp# Analizador de espectro en tiempo real
        └── BandPanel.h/.cpp       # Controles numéricos precisos por banda
```

## Notas sobre el diseño técnico

- **Bandas "flexibles" con parámetros fijos**: para máxima compatibilidad con
  hosts VST3/AU (incluido Studio One), el plugin expone 12 bandas fijas en el
  árbol de parámetros, cada una con un flag "Active". Esto da la sensación de
  bandas ilimitadas (como Pro-Q) sin los problemas de compatibilidad que trae
  crear parámetros dinámicos en tiempo real. Puedes subir `maxBands` en
  `PluginProcessor.h` si quieres más de 12.
- **Proportional Q**: es una aproximación matemática razonable del
  comportamiento de las consolas API (el Q crece con el valor absoluto de la
  ganancia). No es una réplica de circuito exacta, ya que el circuito real es
  propiedad de API.
- El saturador usa sobremuestreo x2 con `juce::dsp::Oversampling` para evitar
  aliasing del waveshaper `tanh`.

## Personalización rápida

- Cambiar el color de acento: edita `AnalogColours::accent` en
  `Source/GUI/LookAndFeel.h`.
- Cambiar el número máximo de bandas: `maxBands` en `PluginProcessor.h`.
- Ajustar la intensidad del modelo Proportional Q: función
  `EQBand::getEffectiveQ()` en `Source/DSP/EQBand.h`.

## Nota importante

Este proyecto fue generado y estructurado completo, pero **no se compiló ni
probó con audio real** en este entorno (no hay DAW, ni Xcode/Visual Studio
disponibles aquí). Al compilarlo por primera vez en tu máquina es normal tener
que resolver algún detalle menor de rutas o versión de compilador — es código
JUCE 7.x estándar e idiomático, pero te recomiendo compilar primero el target
**Standalone** (no requiere Studio One) para probar el sonido y la interfaz
antes de cargarlo como VST3/AU:

```bash
cmake --build build --config Release --target AnalogAPI_EQ_Standalone
```
