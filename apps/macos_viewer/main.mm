#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
#include "io/m3p_parser.h"
#include "io/image_writer.h"
#include "render/backend.h"
#include "render/native_metal/navigation_preview.h"
#include "camera.h"
#include "render/native_metal/general_navigation.h"
#include "presentation.h"
#include <signal.h>
#include <unistd.h>
#include <chrono>
#include <memory>

static NSString* resource(NSString* name) { return [NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:name]; }
static void json(NSDictionary* value, NSString* path) {
    [[NSJSONSerialization dataWithJSONObject:value options:NSJSONWritingPrettyPrinted error:nil] writeToFile:path atomically:YES];
}
#include "service.h"
// Retained one-shot diagnostic entry; the UI uses the persistent service.
static int worker(NSArray<NSString*>* a) {
    if(a.count!=8&&a.count!=9)return 2;
    alarm(300);
    auto scene=mb3d::LoadM3PFile(a[2].UTF8String);
    if(!scene.success){json(@{@"status":@"failed",@"error":@(scene.error.c_str())},a[7]);return 1;}
    std::vector<uint8_t> rgb;
    NSDictionary* result=renderRequest(sceneRequest(scene.header,scene.addon,a[3],a[4],a.count==9?a[8]:@"full",a[5].intValue,0),nullptr,rgb);
    if([result[@"status"] isEqual:@"completed"]&&!mb3d::WritePNG(a[6].UTF8String,rgb.data(),[result[@"width"] intValue],[result[@"height"] intValue],3))
        result=@{@"status":@"failed",@"error":@"Cannot write preview"};
    json(result,a[7]);return [result[@"status"] isEqual:@"completed"]?0:1;
}
// Independent lanes keep navigation's warm GPU state alive when full effects
// are interrupted. A lane has one serial transport owner.
@interface RenderService : NSObject
@property NSTask* task;
@property BOOL ready;
@property NSFileHandle* input;
@property NSFileHandle* output;
@property dispatch_queue_t queue;
@end
@implementation RenderService
- (instancetype)init {if((self=[super init]))self.queue=dispatch_queue_create("mb3d.viewer.transport",DISPATCH_QUEUE_SERIAL);return self;}
@end
@class Viewer;
@interface Canvas : NSView
@property NSImage* image;
@property NSDictionary* frameInfo;
@property NSString* tracePath;
@property NSUInteger drawnFrame;
@property(weak) Viewer* owner;
@end
@interface Viewer : NSObject<NSApplicationDelegate,NSWindowDelegate> {
    mb3d::M3PLoadResult _scene;
    std::unique_ptr<viewer::Camera> _camera;
    viewer::MotionResolution _motionResolution;
}
@property NSWindow* window;
@property Canvas* canvas;
@property NSTextField* status;
@property NSPopUpButton* mode;
@property NSPopUpButton* backend;
@property NSPopUpButton* quality;
@property NSSlider* speed;
@property NSButton* refine;
@property NSString* source;
@property NSString* root;
@property NSString* temp;
@property RenderService* previewService;
@property RenderService* fullService;
@property RenderService* activeService;
@property NSUInteger requestSerial;
@property BOOL busy;
@property BOOL activeFull;
@property unsigned nextPhase;
@property unsigned phaseCount;
@property int previewEdge;
@property BOOL navigationInput;
@property NSDate* started;
@property NSTimer* timer;
@property NSUInteger revision;
@property NSUInteger sceneRevision;
@property double inputSeconds;
@property BOOL p1Check;
@property NSUInteger p1Steps;
@property NSUInteger p1LastAck;
@property NSUInteger presentationEpoch;
@property NSUInteger shownRevision;
@property BOOL dirty;
@property BOOL needsRefine;
@property BOOL interacting;
@property BOOL activeRefinement;
@property BOOL closing;
@property NSDate* lastInput;
@property BOOL diagnosticShown;
- (void)gestureX:(double)x y:(double)y pan:(BOOL)pan;
- (void)moveX:(double)x y:(double)y z:(double)z;
- (void)changed;
- (void)stop:(id)sender;
@end
@implementation Canvas
- (BOOL)acceptsFirstResponder { return YES; }
- (void)drawRect:(NSRect)r {
    [[NSColor colorWithCalibratedWhite:0.055 alpha:1] setFill];NSRectFill(self.bounds);
    if(self.image){
        NSSize s=self.image.size;double k=MIN(self.bounds.size.width/s.width,self.bounds.size.height/s.height);
        NSRect dest=NSMakeRect((self.bounds.size.width-s.width*k)/2,(self.bounds.size.height-s.height*k)/2,s.width*k,s.height*k);
        [self.image drawInRect:dest fromRect:NSZeroRect operation:NSCompositingOperationCopy fraction:1 respectFlipped:YES hints:@{NSImageHintInterpolation:@(NSImageInterpolationHigh)}];
        NSUInteger request=[self.frameInfo[@"request_id"] unsignedIntegerValue];
        if(request&&request!=self.drawnFrame){
            self.drawnFrame=request;
            NSMutableDictionary* event=[self.frameInfo mutableCopy];
            double drawn=CACurrentMediaTime(),input=[event[@"input_seconds"] doubleValue];
            event[@"draw_complete_seconds"]=@(drawn);
            event[@"input_to_draw_seconds"]=input>0&&drawn>=input?@(drawn-input):NSNull.null;
            event[@"present_seconds"]=NSNull.null;event[@"input_to_present_seconds"]=NSNull.null;
            event[@"presentation"]=@"unavailable";event[@"presentation_reason"]=@"AppKit draw completion has no actual display acknowledgment";
            event[@"requested_camera_revision"]=@(self.owner.revision);
            event[@"displayed_camera_revision"]=event[@"camera_revision"];
            event[@"camera_state"]=[event[@"camera_revision"] unsignedIntegerValue]==self.owner.revision?@"current":@"previous_camera";
            if(self.tracePath){
                if(![NSFileManager.defaultManager fileExistsAtPath:self.tracePath])[NSFileManager.defaultManager createFileAtPath:self.tracePath contents:nil attributes:nil];
                NSFileHandle* file=[NSFileHandle fileHandleForWritingAtPath:self.tracePath];[file seekToEndOfFile];
                [file writeData:[NSJSONSerialization dataWithJSONObject:event options:0 error:nil]];[file writeData:[@"\n" dataUsingEncoding:NSUTF8StringEncoding]];[file closeFile];
            }
        }
    }
}
- (void)mouseDown:(NSEvent*)e { [self.window makeFirstResponder:self];self.owner.interacting=YES;self.owner.lastInput=[NSDate date]; }
- (void)rightMouseDown:(NSEvent*)e { [self mouseDown:e]; }
- (void)mouseDragged:(NSEvent*)e { [self.owner gestureX:e.deltaX y:e.deltaY pan:(e.modifierFlags & NSEventModifierFlagShift)!=0]; }
- (void)rightMouseDragged:(NSEvent*)e { [self.owner gestureX:e.deltaX y:e.deltaY pan:YES]; }
- (void)mouseUp:(NSEvent*)e { self.owner.interacting=NO;self.owner.lastInput=[NSDate date]; }
- (void)rightMouseUp:(NSEvent*)e { [self mouseUp:e]; }
- (void)scrollWheel:(NSEvent*)e { [self.owner moveX:0 y:0 z:e.scrollingDeltaY*0.005]; }
- (void)keyDown:(NSEvent*)e {
    NSString* k=e.charactersIgnoringModifiers.lowercaseString;
    if([k isEqual:@"w"]) [self.owner moveX:0 y:0 z:.08];
    else if([k isEqual:@"s"]) [self.owner moveX:0 y:0 z:-.08];
    else if([k isEqual:@"a"]) [self.owner moveX:-.08 y:0 z:0];
    else if([k isEqual:@"d"]) [self.owner moveX:.08 y:0 z:0];
    else if([k isEqual:@"q"]) [self.owner moveX:0 y:-.08 z:0];
    else if([k isEqual:@"e"]) [self.owner moveX:0 y:.08 z:0];
    else if(e.keyCode==53) [self.owner stop:nil];
    else [super keyDown:e];
}
@end
@implementation Viewer
- (NSButton*)button:(NSString*)title action:(SEL)action { return [NSButton buttonWithTitle:title target:self action:action]; }
- (void)applicationDidFinishLaunching:(NSNotification*)n {
    self.temp=[NSTemporaryDirectory() stringByAppendingPathComponent:[@"mb3d-viewer-" stringByAppendingString:NSUUID.UUID.UUIDString]];
    [NSFileManager.defaultManager createDirectoryAtPath:self.temp withIntermediateDirectories:YES attributes:nil error:nil];
    self.root=resource(@"runtime");
    NSArray<NSString*>* args=NSProcessInfo.processInfo.arguments;
    self.p1Check=args.count==5&&[args[1] isEqual:@"--p1-ui-check"];
    if(self.p1Check){self.root=args[3];self.temp=args[4];[NSFileManager.defaultManager createDirectoryAtPath:self.temp withIntermediateDirectories:YES attributes:nil error:nil];alarm(180);}
    signal(SIGPIPE,SIG_IGN);
    self.window=[[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,1200,760) styleMask:NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskMiniaturizable|NSWindowStyleMaskResizable backing:NSBackingStoreBuffered defer:NO];
    self.window.title=@"Mandelbulb Viewer — Preview prototype";self.window.delegate=self;self.window.minSize=NSMakeSize(1150,500);
    self.canvas=[Canvas new];self.canvas.owner=self;
    self.status=[NSTextField labelWithString:@"Open an ordinary .m3p scene"];
    self.status.lineBreakMode=NSLineBreakByTruncatingMiddle;
    self.mode=[NSPopUpButton new];[self.mode addItemsWithTitles:@[@"Orbit",@"Fly"]];
    self.backend=[NSPopUpButton new];[self.backend addItemsWithTitles:@[@"Metal",@"CPU Native"]];self.backend.target=self;self.backend.action=@selector(settings:);
    self.quality=[NSPopUpButton new];[self.quality addItemsWithTitles:@[@"Fast shading",@"Grid",@"Points",@"Full effects"]];[self.quality selectItemAtIndex:2];self.quality.target=self;self.quality.action=@selector(settings:);
    if(generalPreviewEnabled())[self.quality addItemWithTitle:@"Fast geometry (approx.)"];
    self.refine=[NSButton checkboxWithTitle:@"Refine on idle" target:self action:@selector(settings:)];self.refine.state=NSControlStateValueOn;
    self.speed=[NSSlider sliderWithValue:1 minValue:.05 maxValue:3 target:nil action:nil];
    [self.speed.widthAnchor constraintEqualToConstant:90].active=YES;
    NSStackView* bar=[NSStackView stackViewWithViews:@[[self button:@"Open…" action:@selector(open:)],[self button:@"Maps…" action:@selector(maps:)],self.mode,self.backend,self.quality,[self button:@"Reset" action:@selector(reset:)],[self button:@"Stop" action:@selector(stop:)],self.refine,[NSTextField labelWithString:@"Move"],self.speed]];
    bar.spacing=8;
    NSTextField* help=[NSTextField labelWithString:@"Drag: orbit / look · Shift-drag or right-drag: pan · Wheel / W S: forward/back · A D Q E: move · Esc: stop"];
    help.font=[NSFont systemFontOfSize:12];help.textColor=NSColor.secondaryLabelColor;
    NSStackView* stack=[NSStackView stackViewWithViews:@[bar,self.canvas,self.status,help]];
    stack.orientation=NSUserInterfaceLayoutOrientationVertical;stack.alignment=NSLayoutAttributeLeading;stack.spacing=12;stack.translatesAutoresizingMaskIntoConstraints=NO;
    [self.window.contentView addSubview:stack];
    [NSLayoutConstraint activateConstraints:@[[stack.leadingAnchor constraintEqualToAnchor:self.window.contentView.leadingAnchor constant:16],[stack.trailingAnchor constraintEqualToAnchor:self.window.contentView.trailingAnchor constant:-16],[stack.topAnchor constraintEqualToAnchor:self.window.contentView.topAnchor constant:14],[stack.bottomAnchor constraintEqualToAnchor:self.window.contentView.bottomAnchor constant:-14],[self.canvas.widthAnchor constraintEqualToAnchor:stack.widthAnchor],[self.status.widthAnchor constraintEqualToAnchor:stack.widthAnchor],[self.canvas.heightAnchor constraintGreaterThanOrEqualToConstant:280]]];
    [self.canvas setContentHuggingPriority:1 forOrientation:NSLayoutConstraintOrientationVertical];
    [self.window center];[self.window makeKeyAndOrderFront:nil];[NSApp activateIgnoringOtherApps:YES];
    self.timer=[NSTimer timerWithTimeInterval:(1.0/60.0) target:self selector:@selector(tick:) userInfo:nil repeats:YES];
    [NSRunLoop.mainRunLoop addTimer:self.timer forMode:NSRunLoopCommonModes];
    if(self.p1Check)self.refine.state=NSControlStateValueOff;
    [self load:self.p1Check?args[2]:resource(@"demo.m3p")];
}
- (void)load:(NSString*)path {
    auto candidate=mb3d::LoadM3PFile(path.UTF8String);
    if(!candidate.success){self.status.stringValue=@(candidate.error.c_str());return;}
    // Camera adapter preserves the authored optic; no conversion to mode zero.
    if(!viewer::Camera::supports(candidate.header)){self.status.stringValue=@"Supported cameras: mono standard, planar and spherical. Scene was not changed.";return;}
    try { auto cam=std::make_unique<viewer::Camera>(candidate.header);[self stop:nil];_scene=candidate;_camera=std::move(cam); }
    catch(const std::exception& e){self.status.stringValue=@(e.what());return;}
    self.sceneRevision++;self.navigationInput=NO;_motionResolution={};self.source=path;self.canvas.image=nil;[self.canvas setNeedsDisplay:YES];
    self.window.title=[@"Mandelbulb Viewer — " stringByAppendingString:path.lastPathComponent];[self changed];
}
- (void)open:(id)sender {
    NSOpenPanel* p=[NSOpenPanel openPanel];p.allowedContentTypes=@[[UTType typeWithFilenameExtension:@"m3p"]];p.allowsMultipleSelection=NO;
    [p beginSheetModalForWindow:self.window completionHandler:^(NSModalResponse r){if(r==NSModalResponseOK)[self load:p.URL.path];}];
}
- (void)maps:(id)sender {
    NSOpenPanel* p=[NSOpenPanel openPanel];p.canChooseFiles=NO;p.canChooseDirectories=YES;p.message=@"Choose resource root containing M3Maps";
    [p beginSheetModalForWindow:self.window completionHandler:^(NSModalResponse r){if(r==NSModalResponseOK){self.root=p.URL.path;[self settings:nil];}}];
}
- (void)reset:(id)sender { if(self.source)[self load:self.source]; }
- (void)settings:(id)sender {
    if(self.backend.indexOfSelectedItem==1&&self.quality.indexOfSelectedItem==4)[self.quality selectItemAtIndex:2];
    if(self.quality.numberOfItems>4)[self.quality itemAtIndex:4].enabled=self.backend.indexOfSelectedItem==0;
    self.sceneRevision++;self.navigationInput=NO;_motionResolution={};[self stop:nil];[self changed];
}
- (void)changed { if(!_camera)return;
    // Detach the full-effects lane immediately. Its reply cannot release or
    // overwrite a newer preview request; navigation keeps its resident service.
    if(self.busy&&self.activeFull&&self.activeRefinement){
        NSTask* old=self.activeService.task;if(old.running)kill(old.processIdentifier,SIGKILL);
        self.fullService=nil;self.activeService=nil;self.busy=NO;self.requestSerial++;
    }
    if(self.busy&&generalPreviewEnabled()&&viewer::cancelIdleNavigation(self.activeFull,self.activeRefinement,self.activeService.ready)){
        NSTask* job=self.activeService.task;if(job.running)kill(job.processIdentifier,SIGUSR1);
    }
    self.nextPhase=0;self.phaseCount=0;
    self.revision++;self.inputSeconds=CACurrentMediaTime();self.dirty=YES;self.needsRefine=YES;self.lastInput=[NSDate date];
    self.status.stringValue=@"Moving · updating preview…";
    [self tick:nil];
}
- (void)gestureX:(double)x y:(double)y pan:(BOOL)pan {
    if(!_camera)return;
    if(pan)_camera->move(-x*.002*self.speed.doubleValue,y*.002*self.speed.doubleValue,0);
    else _camera->rotate(-x*.006,-y*.006,self.mode.indexOfSelectedItem==1);
    self.navigationInput=YES;[self changed];
}
- (void)moveX:(double)x y:(double)y z:(double)z { if(!_camera)return;double s=self.speed.doubleValue;_camera->move(x*s,y*s,z*s);self.navigationInput=YES;[self changed]; }
- (void)terminateService {
    NSTask* job=self.activeService.task;
    if(job.running){[job terminate];dispatch_after(dispatch_time(DISPATCH_TIME_NOW,250*NSEC_PER_MSEC),dispatch_get_main_queue(),^{if(job.running)kill(job.processIdentifier,SIGKILL);});}
}
- (void)stop:(id)sender {
    self.presentationEpoch++;self.revision++;self.dirty=NO;self.needsRefine=NO;self.nextPhase=0;self.phaseCount=0;self.previewEdge=0;
    // Explicit Stop/settings may terminate a busy service; ordinary motion does not.
    if(self.busy)[self terminateService];
    self.status.stringValue=@"Stopped · last image retained";
}
- (void)tick:(id)sender {
    if(self.closing||!_camera)return;
    if(self.busy){
        if(!self.diagnosticShown && -self.started.timeIntervalSinceNow>180){
            self.diagnosticShown=YES;[self stop:nil];[self terminateService];
            self.status.stringValue=@"Slow request (>180 s) · stopped; inspect service.log";
        }
        return;
    }
    if(self.p1Check&&self.canvas.drawnFrame>self.p1LastAck){
        self.p1LastAck=self.canvas.drawnFrame;
        if(self.p1Steps==20){[NSApp terminate:nil];return;}
        ++self.p1Steps;[self gestureX:.05 y:.025 pan:NO];return;
    }
    BOOL idle=!self.interacting && -self.lastInput.timeIntervalSinceNow>.3;
    if(self.dirty)[self launch:self.navigationInput?_motionResolution.edge():640];
    else if(self.needsRefine&&idle&&(self.previewEdge<640||self.refine.state==NSControlStateValueOn||self.quality.indexOfSelectedItem==3))[self launch:640];
}
- (BOOL)ensureService:(RenderService*)service {
    if(service.task.running)return YES;
    service.task=nil;service.ready=NO;
    [service.input closeFile];[service.output closeFile];
    NSPipe* input=[NSPipe pipe];NSPipe* output=[NSPipe pipe];
    NSTask* job=[NSTask new];job.executableURL=[NSURL fileURLWithPath:NSBundle.mainBundle.executablePath];job.arguments=@[@"--service"];
    job.standardInput=input;job.standardOutput=output;
    NSString* log=[self.temp stringByAppendingPathComponent:@"service.log"];
    if(![NSFileManager.defaultManager fileExistsAtPath:log])[NSFileManager.defaultManager createFileAtPath:log contents:nil attributes:nil];
    NSFileHandle* errors=[NSFileHandle fileHandleForWritingAtPath:log];[errors seekToEndOfFile];job.standardError=errors;
    NSError* error=nil;
    if(![job launchAndReturnError:&error]){[errors closeFile];self.status.stringValue=error.localizedDescription;return NO;}
    [errors closeFile];
    service.task=job;service.input=input.fileHandleForWriting;service.output=output.fileHandleForReading;
    // Parent does not own the child's pipe ends: EOF must wake the reader on exit.
    [input.fileHandleForReading closeFile];[output.fileHandleForWriting closeFile];
    return YES;
}
- (void)launch:(int)edge {
    NSUInteger revision=self.revision,epoch=self.presentationEpoch;
    BOOL refinement=!self.dirty;self.activeRefinement=refinement;
    const BOOL motion=!refinement&&self.navigationInput;
    auto pass=viewer::previewPass(static_cast<viewer::Quality>(self.quality.indexOfSelectedItem),refinement,self.nextPhase,self.phaseCount,self.previewEdge,edge);
    unsigned phase=pass.phase;
    NSString* quality=@[@"draft",@"grid",@"points",@"full",@"fastgeometry"][static_cast<unsigned>(pass.quality)];
    self.activeFull=pass.quality==viewer::Quality::Full;
    RenderService* service;
    if(self.activeFull){if(!self.fullService)self.fullService=[RenderService new];service=self.fullService;}
    else {if(!self.previewService)self.previewService=[RenderService new];service=self.previewService;}
    if(![self ensureService:service]){self.dirty=NO;self.needsRefine=NO;return;}
    self.activeService=service;
    if(self.activeFull)self.status.stringValue=@"Full effects · rendering… · accumulated preview retained";
    NSUInteger serial=++self.requestSerial;
    auto header=_scene.header;_camera->write(header);
    NSMutableDictionary* request=[sceneRequest(header,_scene.addon,self.root,self.backend.indexOfSelectedItem==0?@"metal":@"cpu",quality,edge,phase) mutableCopy];
    [request addEntriesFromDictionary:@{@"request_id":@(serial),@"scene_revision":@(self.sceneRevision),@"camera_revision":@(revision),@"quality_revision":@(self.quality.indexOfSelectedItem+1),@"epoch":@(epoch),@"input_seconds":@(self.inputSeconds),@"submit_seconds":@(CACurrentMediaTime())}];
    self.dirty=NO;self.busy=YES;if(!phase)self.started=[NSDate date];self.diagnosticShown=NO;
    NSTask* job=service.task;NSFileHandle* input=service.input;NSFileHandle* output=service.output;
    __weak Viewer* weak=self;
    dispatch_async(service.queue,^{@autoreleasepool {
        std::vector<uint8_t> rgb;
        NSDictionary* info=sendPacket(input.fileDescriptor,request)?receivePacket(output.fileDescriptor,rgb):nil;
        BOOL disconnected=!info;
        if(disconnected&&job.running){kill(job.processIdentifier,SIGKILL);[job waitUntilExit];}
        if(!info)info=@{@"status":@"failed",@"error":@"Render service disconnected; inspect service.log"};
        if(info&&[info[@"protocol"] intValue]!=1)info=@{@"status":@"failed",@"error":@"Private reply version mismatch"};
        for(NSString* key in @[@"request_id",@"scene_revision",@"camera_revision",@"quality_revision",@"epoch"])
            if(info&&![info[key] isEqual:request[key]]){rgb.clear();info=@{@"status":@"failed",@"error":@"Private reply identity mismatch"};break;}
        NSImage* image=nil;
        if([info[@"status"] isEqual:@"completed"]){
            int w=[info[@"width"] intValue],h=[info[@"height"] intValue];
            if(w<1||h<1||w>1600||h>1600||rgb.size()!=size_t(w)*h*3)
                info=@{@"status":@"failed",@"error":@"Invalid render service frame"};
            else {
                NSBitmapImageRep* bitmap=[[NSBitmapImageRep alloc] initWithBitmapDataPlanes:nil pixelsWide:w pixelsHigh:h bitsPerSample:8 samplesPerPixel:3 hasAlpha:NO isPlanar:NO colorSpaceName:NSDeviceRGBColorSpace bytesPerRow:w*3 bitsPerPixel:24];
                memcpy(bitmap.bitmapData,rgb.data(),rgb.size());image=[[NSImage alloc] initWithSize:NSMakeSize(w,h)];[image addRepresentation:bitmap];
            }
        }
        NSMutableDictionary* joined=[info mutableCopy];
        for(NSString* k in @[@"request_id",@"scene_revision",@"camera_revision",@"quality_revision",@"epoch",@"input_seconds",@"submit_seconds"])joined[k]=request[k];
        joined[@"protocol"]=@1;joined[@"presentation"]=@"unpresented";joined[@"present_seconds"]=NSNull.null;
        info=joined;
        dispatch_async(dispatch_get_main_queue(),^{
            service.ready=info!=nil&&job.running;
            if(!job.running){service.task=nil;[input closeFile];[output closeFile];}
            Viewer* s=weak;
            if(s){NSString* path=[s.temp stringByAppendingPathComponent:@"requests.jsonl"];
                if(![NSFileManager.defaultManager fileExistsAtPath:path])[NSFileManager.defaultManager createFileAtPath:path contents:nil attributes:nil];
                NSFileHandle* file=[NSFileHandle fileHandleForWritingAtPath:path];[file seekToEndOfFile];
                [file writeData:[NSJSONSerialization dataWithJSONObject:info options:0 error:nil]];[file writeData:[@"\n" dataUsingEncoding:NSUTF8StringEncoding]];[file closeFile];}
            if(!s||s.closing||serial!=s.requestSerial)return;s.busy=NO;s.activeService=nil;
            BOOL current=revision==s.revision&&epoch==s.presentationEpoch;
            BOOL cancelled=[info[@"status"] isEqual:@"cancelled"];
            if(current){
                if(cancelled){s.dirty=YES;s.phaseCount=0;s.nextPhase=0;s.needsRefine=YES;}
                else {
                s.phaseCount=[info[@"phases"] unsignedIntValue];s.nextPhase=phase+1;s.previewEdge=edge;
                s.needsRefine=[info[@"status"] isEqual:@"completed"]&&
                    ![quality isEqual:@"full"];
                }
            }
            if(motion&&image&&epoch==s.presentationEpoch)
                s->_motionResolution.observe([info[@"seconds"] doubleValue],[info[@"compile_seconds"] doubleValue]>0||[info[@"archive_misses"] unsignedIntValue]>0);
            if(viewer::shouldPresent(revision,s.revision,s.shownRevision,epoch,s.presentationEpoch,refinement)){
                if(image){
                    s.shownRevision=revision;s.canvas.frameInfo=info;s.canvas.tracePath=[s.temp stringByAppendingPathComponent:@"presentation.jsonl"];s.canvas.image=image;[s.canvas setNeedsDisplay:YES];
                    NSString* progress=[info[@"phases"] unsignedIntValue]>1?[NSString stringWithFormat:@" · points %.1f%% (%u/%@)",100.*[info[@"covered"] doubleValue]/([info[@"width"] doubleValue]*[info[@"height"] doubleValue]),phase+1,info[@"phases"]]:@"";
                    NSString* stage=[quality isEqual:@"full"]?@"Full effects":motion?@"Moving preview":@"Refining preview";
                    s.status.stringValue=[NSString stringWithFormat:@"%@ · %@ · %@×%@ · %.0f ms%@",stage,info[@"backend"],info[@"width"],info[@"height"],1000.*[info[@"seconds"] doubleValue],progress];
                    if(!current)s.status.stringValue=[s.status.stringValue stringByAppendingString:@" · previous camera; updating"];
                }else if(current&&!cancelled){
                    s.needsRefine=NO;s.status.stringValue=[NSString stringWithFormat:@"Render failed: %@ · %@",info[@"error"],s.temp];
                    json(info,[s.temp stringByAppendingPathComponent:@"last-error.json"]);
                    json(request,[s.temp stringByAppendingPathComponent:@"last-request.json"]);
                }
            }
            [s tick:nil];
        });
    }});
}
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)app {return YES;}
- (void)applicationWillTerminate:(NSNotification*)n {
    self.closing=YES;[self.timer invalidate];
    for(RenderService* service in @[self.previewService?:[NSNull null],self.fullService?:[NSNull null]]){
        if(![service isKindOfClass:RenderService.class])continue;
        if(service.task.running){kill(service.task.processIdentifier,SIGKILL);[service.task waitUntilExit];}
    }
    /* Keep service diagnostics for inspection. */
}
@end
int main(int argc,const char* argv[]) {
    @autoreleasepool {
        NSArray<NSString*>* args=NSProcessInfo.processInfo.arguments;
        if(argc>1&&[args[1] isEqual:@"--worker"])return worker(args);
        if(argc>1&&[args[1] isEqual:@"--service"])return service();
        if(argc==7&&[args[1] isEqual:@"--request"]){
            auto scene=mb3d::LoadM3PFile(args[2].UTF8String);if(!scene.success)return 1;
            NSData* data=[NSJSONSerialization dataWithJSONObject:sceneRequest(scene.header,scene.addon,args[3],args[4],args[6],[args[5] intValue],0) options:0 error:nil];
            return transfer(STDOUT_FILENO,(void*)data.bytes,data.length,true)?0:1;
        }
        // The UI exposes general progressive preview by default. Explicit0 keeps
        // the previous restricted/draft selection for diagnostic comparisons.
        setenv("MB3D_VIEWER_GENERAL_PREVIEW","1",0);
        [NSApplication sharedApplication];[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        NSMenu* menu=[NSMenu new];NSMenuItem* app=[NSMenuItem new];[menu addItem:app];NSMenu* submenu=[NSMenu new];[submenu addItemWithTitle:@"Quit Mandelbulb Viewer" action:@selector(terminate:) keyEquivalent:@"q"];app.submenu=submenu;NSApp.mainMenu=menu;
        NSMenuItem* edit=[NSMenuItem new];edit.title=@"Edit";NSMenu* editing=[[NSMenu alloc] initWithTitle:@"Edit"];
        [editing addItemWithTitle:@"Cut" action:@selector(cut:) keyEquivalent:@"x"];
        [editing addItemWithTitle:@"Copy" action:@selector(copy:) keyEquivalent:@"c"];
        [editing addItemWithTitle:@"Paste" action:@selector(paste:) keyEquivalent:@"v"];
        [editing addItemWithTitle:@"Select All" action:@selector(selectAll:) keyEquivalent:@"a"];
        edit.submenu=editing;[menu addItem:edit];
        Viewer* delegate=[Viewer new];NSApp.delegate=delegate;[NSApp run];
    }return 0;
}
