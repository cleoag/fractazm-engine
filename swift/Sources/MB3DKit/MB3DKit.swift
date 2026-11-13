import CMB3D
import Foundation

public struct MB3DError: Error, CustomStringConvertible {
    public let description: String
}

private func withError<T>(_ body: (UnsafeMutablePointer<CChar>, Int) -> T?) throws -> T {
    var buffer = [CChar](repeating: 0, count: 256)
    guard let value = body(&buffer, buffer.count) else { throw MB3DError(description: String(cString: buffer)) }
    return value
}

/// An editable fractal scene. Value-like: `copy()` is cheap, and a render works
/// on the scene it was given, so edit a copy while a render is running.
public final class Scene: @unchecked Sendable {
    let handle: OpaquePointer

    init(handle: OpaquePointer) { self.handle = handle }
    deinit { mb3d_scene_destroy(handle) }

    public convenience init(m3p url: URL) throws {
        self.init(handle: try withError { mb3d_scene_load(url.path, $0, $1) })
    }

    public func copy() -> Scene { Scene(handle: mb3d_scene_clone(handle)) }
    public func save(to url: URL) -> Bool { mb3d_scene_save(handle, url.path) != 0 }
    public func setResourceRoot(_ url: URL) { mb3d_scene_set_resource_root(handle, url.path) }

    public var size: (width: Int, height: Int) {
        var width: Int32 = 0, height: Int32 = 0
        mb3d_scene_get_size(handle, &width, &height)
        return (Int(width), Int(height))
    }

    public var iterations: Int {
        get { Int(mb3d_scene_get_iterations(handle)) }
        set { mb3d_scene_set_iterations(handle, Int32(newValue)) }
    }

    // MARK: Formulas

    /// Names of every formula compiled into the library.
    public static var formulaNames: [String] {
        (0..<mb3d_formula_count()).compactMap { mb3d_formula_name($0).map { String(cString: $0) } }
    }

    public static let slotCount = Int(MB3D_FORMULA_SLOTS)

    public struct Parameter: Sendable { public let name: String; public var value: Double }
    public struct Formula: Sendable {
        public let name: String
        public let iterations: Int
        public let isBuiltin: Bool
        public let parameters: [Parameter]
    }

    public func formula(in slot: Int) -> Formula? {
        var info = MB3DFormulaSlot()
        guard mb3d_scene_get_formula(handle, Int32(slot), &info) != 0 else { return nil }
        let name = withUnsafeBytes(of: info.name) { String(cString: $0.bindMemory(to: CChar.self).baseAddress!) }
        guard !name.isEmpty else { return nil }
        let parameters = (0..<info.param_count).map { index -> Parameter in
            var value = 0.0
            mb3d_scene_get_formula_param(handle, Int32(slot), index, &value)
            return Parameter(name: String(cString: mb3d_scene_formula_param_name(handle, Int32(slot), index)), value: value)
        }
        return Formula(name: name, iterations: Int(info.iterations), isBuiltin: info.builtin != 0, parameters: parameters)
    }

    /// Puts a formula into a hybrid slot using the names and defaults of its .m3f file.
    public func setFormula(m3f url: URL, slot: Int, iterations: Int = 1) throws {
        _ = try withError { mb3d_scene_set_formula_m3f(handle, Int32(slot), url.path, Int32(iterations), $0, $1) != 0 ? true : nil }
    }

    public func clearFormula(slot: Int) { mb3d_scene_clear_formula(handle, Int32(slot)) }

    @discardableResult
    public func setParameter(_ value: Double, index: Int, slot: Int) -> Bool {
        mb3d_scene_set_formula_param(handle, Int32(slot), Int32(index), value) != 0
    }

    public var hybridMode: MB3DHybridMode {
        get { mb3d_scene_get_hybrid_mode(handle) }
        set { mb3d_scene_set_hybrid_mode(handle, newValue) }
    }

    // MARK: Camera, stereo, volume

    public var camera: MB3DCamera? {
        get { var camera = MB3DCamera(); return mb3d_scene_get_camera(handle, &camera) != 0 ? camera : nil }
        set { if var camera = newValue { mb3d_scene_set_camera(handle, &camera) } }
    }

    /// Drag/orbit gesture: angles in radians. `fly` turns the eye instead of orbiting the focus.
    /// Gesture helpers: angles in radians, moves in fractions of the view span.
    /// They keep the camera's start plane out of the fractal (see `mb3d_scene.h`);
    /// false means the step would put the camera into the fractal and was not made.
    @discardableResult
    public func rotate(yaw: Double, pitch: Double, fly: Bool = false) -> Bool {
        mb3d_scene_rotate_camera(handle, yaw, pitch, fly ? 1 : 0) != 0
    }

    /// Turns the camera clockwise about its view direction (the picture turns anticlockwise).
    @discardableResult
    public func roll(_ angle: Double) -> Bool { mb3d_scene_roll_camera(handle, angle) != 0 }

    /// `up` is MB3D's row direction: a positive value moves the camera towards
    /// the bottom edge of the picture, so the picture goes up.
    @discardableResult
    public func move(right: Double = 0, up: Double = 0, forward: Double = 0) -> Bool {
        mb3d_scene_move_camera(handle, right, up, forward) != 0
    }

    /// Sideways in fractions of what the view shows at the surface in front of the
    /// camera, so the surface follows a finger that drags it.
    @discardableResult
    public func pan(right: Double, up: Double) -> Bool { mb3d_scene_pan_camera(handle, right, up) != 0 }

    /// Towards (positive) or away from what the camera looks at: the distance to
    /// the surface in the middle of the picture shrinks or grows by e^amount, so
    /// `log(magnification)` of a pinch shows that surface as much larger.
    @discardableResult
    public func dolly(_ amount: Double) -> Bool { mb3d_scene_dolly_camera(handle, amount) != 0 }

    /// Shape (width / height) of the frames rendered from this scene when the
    /// application fits the camera to another shape than the scene's own.
    public func setNavigationAspect(_ aspect: Double) { mb3d_scene_set_navigation_aspect(handle, aspect) }

    public func stereoPair(separation: Double = 0.03) -> (left: Scene, right: Scene)? {
        guard let left = mb3d_scene_create_eye(handle, MB3D_EYE_LEFT, separation),
              let right = mb3d_scene_create_eye(handle, MB3D_EYE_RIGHT, separation) else { return nil }
        return (Scene(handle: left), Scene(handle: right))
    }

    /// Confines rendering to an axis-aligned box in scene coordinates; nil removes it.
    public var clipVolume: MB3DVolume? {
        get { var volume = MB3DVolume(); return mb3d_scene_get_clip_volume(handle, &volume) != 0 ? volume : nil }
        set {
            if var volume = newValue { mb3d_scene_set_clip_volume(handle, &volume) }
            else { mb3d_scene_set_clip_volume(handle, nil) }
        }
    }

    // MARK: Colours, fog and light

    public struct Color: Sendable, Hashable {
        public var red: UInt8, green: UInt8, blue: UInt8
        public init(red: UInt8, green: UInt8, blue: UInt8) { self.red = red; self.green = green; self.blue = blue }
        init(_ color: MB3DColor) { self.init(red: color.r, green: color.g, blue: color.b) }
        var raw: MB3DColor { MB3DColor(r: red, g: green, b: blue) }
    }

    /// Depth fog: distant surface fades into the background colour, which runs from
    /// `color` at the top of the image to `bottomColor` at the bottom.
    public struct Fog: Sendable, Hashable {
        public var color: Color, bottomColor: Color
        /// 0 (none) to 1; at about 0.19 the far end of the scene is solid fog.
        public var amount: Double
        public init(color: Color, bottomColor: Color, amount: Double) {
            self.color = color; self.bottomColor = bottomColor; self.amount = amount
        }
    }

    /// The first global (directional) light that is on.
    public struct Light: Sendable, Hashable {
        /// Pad position as the viewer sees the picture, each -1...1: (0, 0) shines from
        /// the camera, +x is a light from the right, +y from above, 0.5 exactly from the
        /// side and beyond that from behind the scene.
        public var x: Double, y: Double
        /// 0...100; 1 is the MB3D default.
        public var brightness: Double
        /// Hard shadows of this light; only a final-quality render shows them.
        public var hardShadow: Bool
        public init(x: Double = 0, y: Double = 0, brightness: Double = 1, hardShadow: Bool = false) {
            self.x = x; self.y = y; self.brightness = brightness; self.hardShadow = hardShadow
        }
    }

    public static let paletteCount = Int(MB3D_PALETTE_COLORS)

    /// The ten surface colours. Assigning another number of colours changes nothing.
    public var palette: [Color] {
        get {
            var colors = [MB3DColor](repeating: MB3DColor(), count: Scene.paletteCount)
            mb3d_scene_get_palette(handle, &colors)
            return colors.map { Color($0) }
        }
        set {
            guard newValue.count == Scene.paletteCount else { return }
            mb3d_scene_set_palette(handle, newValue.map(\.raw))
        }
    }

    /// Where each colour sits along the palette, 0...1, ascending. Many scenes pack
    /// most colours at the end; `(0..<10).map { Double($0) / 10 }` spreads them evenly.
    public var palettePositions: [Double] {
        get {
            var positions = [Double](repeating: 0, count: Scene.paletteCount)
            mb3d_scene_get_palette_positions(handle, &positions)
            return positions
        }
        set {
            guard newValue.count == Scene.paletteCount else { return }
            mb3d_scene_set_palette_positions(handle, newValue)
        }
    }

    public static let paletteRangeLimits = (-10900.0 / 32767.0)...(16.0 / 9.0 - 10900.0 / 32767.0)

    /// The part of the colouring value (0...1 over the surface) the palette is laid
    /// over: moving both ends shifts the colours, `end - start` stretches them and
    /// `end < start` reverses them. Both are clamped to `paletteRangeLimits`.
    public var paletteRange: (start: Double, end: Double) {
        get {
            var start = 0.0, end = 0.0
            mb3d_scene_get_palette_range(handle, &start, &end)
            return (start, end)
        }
        set { mb3d_scene_set_palette_range(handle, newValue.start, newValue.end) }
    }

    public var fog: Fog {
        get {
            var fog = MB3DFog()
            mb3d_scene_get_fog(handle, &fog)
            return Fog(color: Color(fog.color), bottomColor: Color(fog.color_bottom), amount: fog.amount)
        }
        set {
            var fog = MB3DFog(color: newValue.color.raw, color_bottom: newValue.bottomColor.raw, amount: newValue.amount)
            mb3d_scene_set_fog(handle, &fog)
        }
    }

    /// Nil when no global light is on. Assigning a light then switches on the first
    /// light that is off, as a white default light; assigning nil changes nothing.
    public var light: Light? {
        get {
            var light = MB3DLight()
            guard mb3d_scene_get_light(handle, &light) != 0 else { return nil }
            return Light(x: light.x, y: light.y, brightness: light.brightness, hardShadow: light.hard_shadow != 0)
        }
        set {
            guard let newValue else { return }
            var light = MB3DLight(x: newValue.x, y: newValue.y, brightness: newValue.brightness,
                                  hard_shadow: newValue.hardShadow ? 1 : 0)
            mb3d_scene_set_light(handle, &light)
        }
    }

    /// Strength of the ambient light: 0 none, 1 the MB3D default, 3 the maximum.
    public var ambient: Double {
        get { var amount = 0.0; mb3d_scene_get_ambient(handle, &amount); return amount }
        set { mb3d_scene_set_ambient(handle, newValue) }
    }
}

/// One delivered pass: straight RGBA8 (alpha 0 where no surface was hit) and packed depth.
public struct Frame: Sendable {
    public let rgba: Data
    public let depth: [UInt16]
    public let width: Int, height: Int
    public let pass: Int, passCount: Int
    public let usedMetal: Bool
    /// Wall time of this pass inside the library.
    public let seconds: Double
    /// Metal pipelines this pass compiled (cold) or loaded from the disk cache; 0 when warm.
    public let pipelinesCompiled: Int, pipelinesLoaded: Int
    /// Time the GPU spent executing this pass's commands, and their count; 0 on the CPU.
    public let gpuSeconds: Double, gpuCommands: Int
    /// Why a pass that could have used Metal ran on the CPU; nil when it ran where requested.
    public let cpuFallbackReason: String?
    public var isFinal: Bool { pass == passCount - 1 }

    /// This frame of `source` redrawn as the camera of `target` would see the same
    /// surface, for following a gesture at display rate while the next frame renders.
    /// The scenes may differ only in the camera. `coverage` is the share of pixels a
    /// frame pixel landed on; the rest is filled from neighbouring content. Nil for a
    /// projection the navigation camera does not model.
    public func reprojected(from source: Scene, to target: Scene) -> (rgba: Data, coverage: Double)? {
        guard depth.count == width * height, rgba.count == depth.count * 4 else { return nil }
        var out = Data(count: rgba.count)
        let coverage = out.withUnsafeMutableBytes { output in
            rgba.withUnsafeBytes { input in
                mb3d_scene_reproject_frame(source.handle, target.handle,
                    input.bindMemory(to: UInt8.self).baseAddress, depth, Int32(width), Int32(height),
                    output.bindMemory(to: UInt8.self).baseAddress)
            }
        }
        return coverage < 0 ? nil : (out, coverage)
    }
}

/// One of the parts a final frame can be rendered in: every `stride`-th pixel
/// of the frame, starting at pixel (`x`, `y`). See "Lattice frames" in `mb3d_scene.h`.
public struct Lattice: Sendable, Equatable {
    public let stride: Int, x: Int, y: Int
    public init(stride: Int, x: Int, y: Int) { self.stride = stride; self.x = x; self.y = y }

    /// The `stride * stride` lattices of a stride (2 to 4), in the order that covers
    /// the picture ever more finely.
    public static func sequence(stride: Int) -> [Lattice] {
        (0..<max(0, stride * stride)).compactMap { index in
            var x: Int32 = 0, y: Int32 = 0
            return mb3d_lattice_offset(Int32(stride), Int32(index), &x, &y) != 0 ? Lattice(stride: stride, x: Int(x), y: Int(y)) : nil
        }
    }
}

/// A picture that fills as its lattice frames arrive. After every part it is
/// complete: pixels not rendered yet are interpolated from those around them.
public final class LatticePicture {
    public let width: Int, height: Int, stride: Int
    /// How many of the `stride * stride` lattices are in the picture.
    public private(set) var parts = 0
    public var isComplete: Bool { parts == stride * stride }
    /// Time of the parts so far, as `Frame` reports it.
    public private(set) var seconds = 0.0, gpuSeconds = 0.0, gpuCommands = 0
    private var rgba: Data
    private var depth: [UInt16]
    private var known: [UInt8]

    public init(width: Int, height: Int, stride: Int) {
        self.width = width; self.height = height; self.stride = stride
        rgba = Data(count: width * height * 4)
        depth = [UInt16](repeating: 0, count: width * height)
        known = [UInt8](repeating: 0, count: width * height)
    }

    /// Puts in a frame rendered for `lattice`; false when it does not belong to this picture.
    @discardableResult
    public func add(_ frame: Frame, lattice: Lattice) -> Bool {
        guard lattice.stride == stride, frame.depth.count == frame.width * frame.height,
              frame.rgba.count == frame.depth.count * 4 else { return false }
        let count = rgba.withUnsafeMutableBytes { picture in
            frame.rgba.withUnsafeBytes { part in
                mb3d_lattice_merge(picture.bindMemory(to: UInt8.self).baseAddress, &depth, &known,
                    Int32(width), Int32(height), part.bindMemory(to: UInt8.self).baseAddress, frame.depth,
                    Int32(frame.width), Int32(frame.height), Int32(stride), Int32(lattice.x), Int32(lattice.y))
            }
        }
        guard count >= 0 else { return false }
        parts = Int(count)
        seconds += frame.seconds; gpuSeconds += frame.gpuSeconds; gpuCommands += frame.gpuCommands
        return true
    }

    /// The picture so far as one frame of the full size; it is the result once `isComplete`.
    public func frame() -> Frame {
        Frame(rgba: rgba, depth: depth, width: width, height: height, pass: max(0, parts - 1), passCount: stride * stride,
              usedMetal: true, seconds: seconds, pipelinesCompiled: 0, pipelinesLoaded: 0,
              gpuSeconds: gpuSeconds, gpuCommands: gpuCommands, cpuFallbackReason: nil)
    }
}

public struct RenderSettings: Sendable {
    public var width = 0, height = 0
    public var quality = MB3D_QUALITY_PREVIEW
    public var backend = MB3D_BACKEND_AUTO
    /// `metal_native_scene.metallib` from the app bundle; nil renders on the CPU.
    public var metalLibrary: URL?
    public var previewFirstEdge = 0
    public var precision = MB3D_PRECISION_DEFAULT
    /// Progress of a final-quality render, 0 to 1, never decreasing; called off the
    /// main thread. With it, a frame of 640 px or more is preceded by a quarter-size
    /// frame in full quality.
    public var onProgress: (@Sendable (Double) -> Void)?
    /// Renders this part of the frame instead of the whole (final quality on Metal
    /// only): the frame delivered has the lattice's pixels. Nil: the whole frame.
    public var lattice: Lattice?
    /// Told the stage each pass begins (`pass` of `passCount`, 0-based); called on
    /// the rendering thread. With `onProgress` the quarter-size pass goes through
    /// the stages of the full pass first.
    public var onStage: (@Sendable (_ stage: MB3DRenderStage, _ pass: Int, _ passCount: Int) -> Void)?
    /// Metal only: the scene's GPU programs call its formulas instead of
    /// repeating them at every place of use, so a scene seen for the first
    /// time starts several times sooner; the rendering speed is the same. The
    /// programs are kept apart from those made without it: use one value for a scene.
    public var quickPipelines = false
    public init() {}
}

public enum Renderer {
    /// Why this device cannot render a final frame of `settings` (its size and
    /// lattice) on Metal, or nil when it can: the scene's capabilities and the
    /// memory the frame needs are checked without rendering. It prepares the
    /// scene as a render does; call it off the main thread.
    public static func refusal(of scene: Scene, settings: RenderSettings) -> String? {
        guard let library = settings.metalLibrary else { return "No shader library" }
        var request = MB3DRenderRequest()
        request.width = Int32(settings.width)
        request.height = Int32(settings.height)
        request.quick_pipelines = settings.quickPipelines ? 1 : 0
        if let lattice = settings.lattice {
            request.lattice_stride = Int32(lattice.stride)
            request.lattice_x = Int32(lattice.x)
            request.lattice_y = Int32(lattice.y)
        }
        var message = [CChar](repeating: 0, count: 256)
        let status = library.path.withCString { path -> MB3DRenderStatus in
            request.metal_library = path
            return mb3d_scene_admits(scene.handle, &request, &message, message.count)
        }
        return status == MB3D_RENDER_SUCCESS ? nil : String(cString: message)
    }

    /// Renders `scene` off the caller's thread. A preview yields growing passes; the
    /// last frame is the result. Ending the iteration early cancels the render.
    public static func frames(of scene: Scene, settings: RenderSettings) -> AsyncThrowingStream<Frame, Error> {
        stream(scene: scene, settings: settings, session: nil)
    }
}

/// A renderer for many frames, such as interactive navigation. It keeps the Metal
/// device, the shader library and the passed setup checks between renders, so a
/// frame does not load the library again. Images are identical to `Renderer.frames`.
/// One session renders one frame at a time.
public final class RenderSession: @unchecked Sendable {
    let handle: OpaquePointer

    /// `metalLibrary` is `metal_native_scene.metallib` from the app bundle and replaces
    /// `RenderSettings.metalLibrary`; nil renders on the CPU.
    public init(metalLibrary: URL?) throws {
        handle = try withError { error, size in
            metalLibrary.map { $0.path.withCString { mb3d_renderer_create($0, error, size) } }
                ?? mb3d_renderer_create(nil, error, size)
        }
    }

    deinit { mb3d_renderer_destroy(handle) }

    public func frames(of scene: Scene, settings: RenderSettings) -> AsyncThrowingStream<Frame, Error> {
        stream(scene: scene, settings: settings, session: self)
    }
}

private func stream(scene: Scene, settings: RenderSettings, session: RenderSession?) -> AsyncThrowingStream<Frame, Error> {
    AsyncThrowingStream { continuation in
        let token = CancelToken()
        let snapshot = scene.copy()
        let work = Thread {
            var request = MB3DRenderRequest()
            request.width = Int32(settings.width)
            request.height = Int32(settings.height)
            request.quality = settings.quality
            request.backend = settings.backend
            request.preview_first_edge = Int32(settings.previewFirstEdge)
            request.quick_pipelines = settings.quickPipelines ? 1 : 0
            if let lattice = settings.lattice {
                request.lattice_stride = Int32(lattice.stride)
                request.lattice_x = Int32(lattice.x)
                request.lattice_y = Int32(lattice.y)
            }
            request.precision = settings.precision
            let box = Unmanaged.passRetained(Sink(continuation, progress: settings.onProgress, stage: settings.onStage))
            if settings.onStage != nil {
                request.stage = { stage, pass, count, user in
                    guard let user else { return }
                    Unmanaged<Sink>.fromOpaque(user).takeUnretainedValue().stage?(stage, Int(pass), Int(count))
                }
                request.stage_user = box.toOpaque()
            }
            if settings.onProgress != nil {
                request.progress = { fraction, user in
                    guard let user else { return }
                    Unmanaged<Sink>.fromOpaque(user).takeUnretainedValue().progress?(fraction)
                }
                request.progress_user = box.toOpaque()
            }
            var message = [CChar](repeating: 0, count: 256)
            let deliver: MB3DFrameCallback = { frame, user in
                guard let frame = frame?.pointee, let user else { return }
                let pixels = Int(frame.width) * Int(frame.height)
                let depth = frame.depth.map { Array(UnsafeBufferPointer(start: $0, count: pixels)) } ?? []
                Unmanaged<Sink>.fromOpaque(user).takeUnretainedValue().continuation.yield(Frame(
                    rgba: Data(bytes: frame.rgba, count: pixels * 4), depth: depth,
                    width: Int(frame.width), height: Int(frame.height),
                    pass: Int(frame.pass), passCount: Int(frame.pass_count),
                    usedMetal: frame.backend == MB3D_BACKEND_METAL,
                    seconds: frame.render_seconds,
                    pipelinesCompiled: Int(frame.pipelines_compiled), pipelinesLoaded: Int(frame.pipelines_loaded),
                    gpuSeconds: frame.gpu_seconds, gpuCommands: Int(frame.gpu_commands),
                    cpuFallbackReason: frame.cpu_fallback_reason.map { String(cString: $0) }))
            }
            let render = { (library: UnsafePointer<CChar>?) -> MB3DRenderStatus in
                request.metal_library = library
                if let session {
                    return mb3d_renderer_render(session.handle, snapshot.handle, &request, token.raw, deliver,
                                                box.toOpaque(), &message, message.count)
                }
                return mb3d_scene_render(snapshot.handle, &request, token.raw, deliver, box.toOpaque(), &message, message.count)
            }
            let status = settings.metalLibrary.map { $0.path.withCString(render) } ?? render(nil)
            box.release()
            if status == MB3D_RENDER_SUCCESS || status == MB3D_RENDER_CANCELLED { continuation.finish() }
            else { continuation.finish(throwing: MB3DError(description: String(cString: message))) }
        }
        // Both closures retain the token, so a late cancel never touches freed memory.
        continuation.onTermination = { @Sendable reason in
            if case .cancelled = reason { mb3d_cancel_request(token.raw) }
        }
        work.stackSize = 16 << 20
        work.start()
    }
}

private final class CancelToken: @unchecked Sendable {
    let raw = mb3d_cancel_create()
    deinit { mb3d_cancel_destroy(raw) }
}

private final class Sink {
    let continuation: AsyncThrowingStream<Frame, Error>.Continuation
    let progress: (@Sendable (Double) -> Void)?
    let stage: (@Sendable (MB3DRenderStage, Int, Int) -> Void)?
    init(_ continuation: AsyncThrowingStream<Frame, Error>.Continuation, progress: (@Sendable (Double) -> Void)?,
         stage: (@Sendable (MB3DRenderStage, Int, Int) -> Void)?) {
        self.continuation = continuation
        self.progress = progress
        self.stage = stage
    }
}
