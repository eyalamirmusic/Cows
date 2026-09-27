#include "TouchSurface.h"

#import <AVFoundation/AVFoundation.h>
#import <UIKit/UIKit.h>

#include <map>

@interface CowsTouchLayer : UIView
{
@public
    Cows::TouchControls* controls;
    Cows::TouchSurface* surface;
    std::map<UITouch*, int> ids;
    int nextId;
}
@end

@implementation CowsTouchLayer

- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
    {
        self.multipleTouchEnabled = YES;
        self.userInteractionEnabled = YES;
        self.backgroundColor = UIColor.clearColor;
        self.opaque = NO;
        self.autoresizingMask =
            UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        nextId = 1;
    }
    return self;
}

- (Graphics::Point)positionOf:(UITouch*)touch
{
    auto point = [touch locationInView:self];
    return {(float) point.x, (float) point.y};
}

- (void)touchesBegan:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
    for (UITouch* touch in touches)
    {
        auto id = nextId++;
        ids[touch] = id;
        controls->pointerDown(id, [self positionOf:touch]);
    }
}

- (void)touchesMoved:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
    for (UITouch* touch in touches)
        if (auto found = ids.find(touch); found != ids.end())
            controls->pointerMoved(found->second, [self positionOf:touch]);
}

- (void)endTouches:(NSSet<UITouch*>*)touches
{
    for (UITouch* touch in touches)
    {
        if (auto found = ids.find(touch); found != ids.end())
        {
            controls->pointerUp(found->second);
            ids.erase(found);
        }
    }
}

- (void)touchesEnded:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
    [self endTouches:touches];
}

- (void)touchesCancelled:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
    [self endTouches:touches];
}

- (void)safeAreaInsetsDidChange
{
    [super safeAreaInsetsDidChange];
    [self reportSafeArea];
}

- (void)layoutSubviews
{
    [super layoutSubviews];
    [self reportSafeArea];
}

- (void)reportSafeArea
{
    auto insets = self.safeAreaInsets;
    surface->onSafeAreaChanged({.top = (float) insets.top,
                                .left = (float) insets.left,
                                .bottom = (float) insets.bottom,
                                .right = (float) insets.right});
}

@end

namespace Cows
{
struct TouchSurface::Native
{
    CowsTouchLayer* layer = nil;
};

TouchSurface::TouchSurface(Graphics::View& host, TouchControls& controls)
    : native(std::make_unique<Native>())
{
    auto* hostView = (UIView*) host.getHandle();
    auto* layer = [[CowsTouchLayer alloc] initWithFrame:hostView.bounds];
    layer->controls = &controls;
    layer->surface = this;
    [hostView addSubview:layer];
    native->layer = layer;
}

TouchSurface::~TouchSurface()
{
    [native->layer removeFromSuperview];
    [native->layer release];
}

void makeSeeThrough(Graphics::View& view)
{
    auto* native = (UIView*) view.getHandle();
    native.opaque = NO;
    native.backgroundColor = UIColor.clearColor;
    native.layer.opaque = NO;
}

void setUpAudioSession()
{
    auto* session = [AVAudioSession sharedInstance];
    [session setCategory:AVAudioSessionCategoryAmbient error:nil];
    [session setActive:YES error:nil];
}
} // namespace Cows
