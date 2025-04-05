// Copyright 2025 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radium/browser/ui/views/dragging/drag_controller.h"

#include <utility>

#include "base/notreached.h"
#include "base/trace_event/typed_macros.h"
#include "radium/browser/ui/views/dragging/drag_context.h"
#include "radium/browser/ui/window_finder.h"
#include "ui/display/screen.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget.h"

#if defined(USE_AURA)
#include "ui/wm/core/window_modality_controller.h"
#endif

namespace {

gfx::Rect GetViewInScreenBounds(const views::View* context) {
  const views::View* view = context;
  gfx::Point view_topleft;
  views::View::ConvertPointToScreen(view, &view_topleft);
  gfx::Rect view_screen_bounds = view->GetLocalBounds();
  view_screen_bounds.Offset(view_topleft.x(), view_topleft.y());
  return view_screen_bounds;
}

}  // namespace

DragController::DragController() = default;
DragController::~DragController() = default;

DragController::Liveness DragController::Init(
    DragContext* source_context,
    views::View* source_view,
    const gfx::Point& mouse_offset,
    int source_view_offset,
    ui::mojom::DragEventSource event_source) {
  event_source_ = event_source;
  source_context_ = source_context;
  start_point_in_screen_ = gfx::Point(source_view_offset, mouse_offset.y());
  views::View::ConvertPointToScreen(source_view, &start_point_in_screen_);

// Do not release capture when transferring capture between widgets on:
// - Desktop Linux
//     Mouse capture is not synchronous on desktop Linux. Chrome makes
//     transferring capture between widgets without releasing capture appear
//     synchronous on desktop Linux, so use that.
#if BUILDFLAG(IS_LINUX)
  can_release_capture_ = false;
#endif

  window_finder_ = std::make_unique<WindowFinder>();
  return Liveness::ALIVE;
}

[[nodiscard]] DragController::Liveness DragController::Drag(
    const gfx::Point& point_in_screen) {
  if (current_state_ == DragState::kNotStarted) {
    if (!CanStartDrag(point_in_screen)) {
      return Liveness::ALIVE;  // User hasn't dragged far enough yet.
    }

    // If any of the tabs have disappeared (e.g. closed or discarded), cancel
    // the drag session. See crbug.com/1445776.
    if (GetViewsMatchingDraggedContents(source_context_).empty()) {
      EndDrag();
      return Liveness::DELETED;
    }

    // // On windows SaveFocus() may trigger a capture lost, which destroys us.
    // {
    //   const Liveness alive = SaveFocus();
    //   if (alive == Liveness::DELETED) {
    //     return Liveness::DELETED;
    //   }
    // }

    current_state_ = DragState::kDraggingTabs;
    StartDrag();
  }

  return ContinueDragging(point_in_screen);
}

void DragController::EndDrag() {
  bring_to_front_timer_.Stop();
  // todo
  DragContext* owning_context =
      attached_context_ ? attached_context_.get() : source_context_.get();
  owning_context->DestroyDragController();
}

void DragController::AttachImpl() {
  const std::vector<raw_ptr<views::View, VectorExperimental>> views =
      GetViewsMatchingDraggedContents(attached_context_);

  // This should be called after ResetSelection() in order to generate
  // bounds correctly. http://crbug.com/836004
  attached_context_->StartedDragging(views);

  // // The size of the dragged tab may have changed. Adjust the x offset so
  // that
  // // ratio of mouse_offset_ to original width is maintained.
  // const int new_x = attached_context_->GetMouseOffset().x();
  // mouse_offset_.set_x(new_x);

  // Make sure the window has capture. This is important so that if activation
  // changes the drag isn't prematurely canceled.
  CHECK_EQ(SetCapture(attached_context_), Liveness::ALIVE);

  // TODO(): 监听视图是否发生了变化。
  // attached_context_tabs_closed_tracker_ =
  //     std::make_unique<DraggedTabsClosedTracker>(
  //         attached_context_->GetTabStripModel(), this);
}

void DragController::AttachToNewContext(
    DragContext* attached_context,
    std::unique_ptr<DragController> controller,
    std::vector<std::unique_ptr<views::View>> views) {
  // We should already have detached by the time we get here.
  CHECK(!attached_context_);
  attached_context_ = attached_context;

  // `this` is owned by `controller`, and not by `attached_context`.
  CHECK_EQ(this, controller.get());
  CHECK(!attached_context_->GetDragController());

  // Transitioning from detached to attached to a new context. Add tabs to
  // the new model.
  CHECK(GetViewsMatchingDraggedContents(attached_context_).empty());

  for (auto& view : views) {
    attached_context_->AttachView(std::exchange(view, nullptr));
  }

  AttachImpl();

  attached_context_->OwnDragController(std::move(controller));
}

bool DragController::CanAttachTo(gfx::NativeWindow window) {
  if (!window) {
    return false;
  }

  if (window == GetAttachedWidget()->GetNativeWindow()) {
    return true;
  }

  // Do not allow dragging into a window with a modal dialog, it causes a
  // weird behavior.  See crbug.com/336691
#if defined(USE_AURA)
  if (wm::GetModalTransient(window)) {
    return false;
  }
#else
#endif  // BUILDFLAG(USE_AURA)

  return attached_context_->CanAttachTo(window);
}

bool DragController::CanStartDrag(const gfx::Point& point_in_screen) const {
  // Determine if the mouse has moved beyond a minimum elasticity distance in
  // any direction from the starting point.
  static const int kMinimumDragDistance = 10;
  int x_offset = abs(point_in_screen.x() - start_point_in_screen_.x());
  int y_offset = abs(point_in_screen.y() - start_point_in_screen_.y());
  return sqrt(pow(static_cast<float>(x_offset), 2) +
              pow(static_cast<float>(y_offset), 2)) > kMinimumDragDistance;
}

[[nodiscard]] DragController::Liveness DragController::ContinueDragging(
    const gfx::Point& point_in_screen) {
  DragContext* target_context = source_context_;
  auto [alive, context] = GetTargetDragContextForPoint(point_in_screen);
  if (alive == Liveness::DELETED) {
    return Liveness::DELETED;
  }
  target_context = context;

  if (target_context != attached_context_) {
    return DragContainerToNewWidget(target_context, point_in_screen);
  }

  if (current_state_ == DragState::kDraggingWindow) {
    // bring_to_front_timer_.Start(
    //     FROM_HERE, base::Milliseconds(750),
    //     base::BindOnce(&TabDragController::BringWindowUnderPointToFront,
    //                    base::Unretained(this), point_in_screen));
  }

  if (current_state_ == DragState::kDraggingTabs) {
    MoveAttached(point_in_screen, false);
  }
  return Liveness::ALIVE;
}

std::tuple<std::unique_ptr<DragController>,
           std::vector<std::unique_ptr<views::View>>>
DragController::Detach(ReleaseCapture release_capture) {
  // Release ownership of the drag controller and mouse capture. When we
  // reattach ownership is transferred.
  std::unique_ptr<DragController> me =
      attached_context_->ReleaseDragController();
  DCHECK_EQ(me.get(), this);

  if (release_capture == RELEASE_CAPTURE) {
    attached_context_->GetWidget()->ReleaseCapture();
  }

  const std::vector<raw_ptr<views::View, VectorExperimental>> views =
      GetViewsMatchingDraggedContents(attached_context_);

  std::vector<std::unique_ptr<views::View>> owned_views(views.size());
  for (size_t index = 0; index < views.size(); ++index) {
    owned_views[index] = attached_context_->DetachView(views[index]);
  }

  attached_context_->DragContextDetached();
  attached_context_ = nullptr;

  return std::make_tuple(std::move(me), std::move(owned_views));
}

void DragController::DetachAndAttachToNewContext(ReleaseCapture release_capture,
                                                 DragContext* target_context) {
  auto [me, owned_tabs] = Detach(release_capture);
  AttachToNewContext(target_context, std::move(me), std::move(owned_tabs));
}

DragController::Liveness DragController::DetachIntoNewWidgetAndRunMoveLoop(
    gfx::Point point_in_screen) {
  DragContext* drag_context = attached_context_->CreateNewDragContext();
  const ReleaseCapture release_capture =
      can_release_capture_ ? RELEASE_CAPTURE : DONT_RELEASE_CAPTURE;
  gfx::Rect anchor_screen_bounds =
      GetViewInScreenBounds(attached_context_->GetAnchorView());
  gfx::Vector2d cusor_point_in_view =
      start_point_in_screen_ - anchor_screen_bounds.origin();

  DetachAndAttachToNewContext(release_capture, drag_context);

  views::Widget* widget = attached_context_->GetWidget();

  // Drag the window relative to `start_point_in_screen_` to pretend that
  // this was the plan all along.
  const gfx::Vector2d drag_offset =
      CalculateDragViewOffsetInWidget() + cusor_point_in_view;
#if (!BUILDFLAG(IS_MAC))
  // Set the window origin before making it visible, to avoid flicker on
  // Windows. See https://crbug.com/394529650
  widget->SetBounds(gfx::Rect(point_in_screen - drag_offset,
                              attached_context_->GetDraggedWindowSize()));
#endif

  widget->SetVisibilityChangedAnimationsEnabled(false);
  widget->Show();
  widget->SetVisibilityChangedAnimationsEnabled(true);

#if BUILDFLAG(IS_MAC)
  // Set the window origin after making it visible, to avoid child windows (such
  // as the find bar) being misplaced on Mac. See https://crbug.com/403129048
  widget->SetBounds(gfx::Rect(point_in_screen - drag_offset,
                              attached_context_->GetDraggedWindowSize()));
#endif

  // Activate may trigger a focus loss, destroying us.
  {
    base::WeakPtr<DragController> ref(weak_factory_.GetWeakPtr());
    widget->Activate();
    if (!ref) {
      return Liveness::DELETED;
    }
  }
  return RunMoveLoop(point_in_screen, drag_offset);
}

bool DragController::DoesDragContextContain(
    DragContext* context,
    const gfx::Point& point_in_screen) const {
  // Make sure the specified screen point is actually within the bounds of the
  // specified context...
  gfx::Rect context_bounds = GetViewInScreenBounds(context->GetAnchorView());
  context_bounds.Inset(context->GetDragAreaInsets());
  return context_bounds.Contains(point_in_screen);
}

DragController::Liveness DragController::DragContainerToNewWidget(
    DragContext* target_context,
    const gfx::Point& point_in_screen) {
  if (!target_context) {
    return DetachIntoNewWidgetAndRunMoveLoop(point_in_screen);
  }
  // todo...
  return Liveness::ALIVE;
}

views::Widget* DragController::GetAttachedWidget() {
  return attached_context_->GetWidget();
}

gfx::Point DragController::GetCursorScreenPoint() {
  return display::Screen::GetScreen()->GetCursorScreenPoint();
}

gfx::Vector2d DragController::CalculateDragViewOffsetInWidget() {
  const gfx::Rect source_tab_bounds =
      attached_context_->GetAnchorView()->bounds();

  const int cursor_offset_within_tab = source_tab_bounds.width();
  gfx::Point desired_cursor_pos_in_widget(
      attached_context_->GetMirroredXInView(source_tab_bounds.x() +
                                            cursor_offset_within_tab),
      source_tab_bounds.y());
  views::View::ConvertPointToWidget(attached_context_,
                                    &desired_cursor_pos_in_widget);
  return desired_cursor_pos_in_widget.OffsetFromOrigin();

  // gfx::Point desired_cursor_pos_in_widget =
  //     GetViewInScreenBounds(attached_context_->GetAnchorView()).origin() +
  //     cursor_pos_in_view;
  // views::View::ConvertPointToWidget(attached_context_,
  //                                   &desired_cursor_pos_in_widget);
  // // Drag the window relative to `start_point_in_screen_` to pretend that
  // // this was the plan all along.
  // const gfx::Vector2d drag_offset =
  //     desired_cursor_pos_in_widget.OffsetFromOrigin();
}

DragController::Liveness DragController::GetLocalProcessWindow(
    const gfx::Point& screen_point,
    bool exclude_dragged_view,
    gfx::NativeWindow* window) {
  std::set<gfx::NativeWindow> exclude;
  if (exclude_dragged_view) {
    gfx::NativeWindow dragged_window =
        attached_context_->GetWidget()->GetNativeWindow();
    if (dragged_window) {
      exclude.insert(dragged_window);
    }
  }

  base::WeakPtr<DragController> ref(weak_factory_.GetWeakPtr());
  *window = window_finder_->GetLocalProcessWindowAtPoint(screen_point, exclude);
  return ref ? Liveness::ALIVE : Liveness::DELETED;
}

std::tuple<DragController::Liveness, DragContext*>
DragController::GetTargetDragContextForPoint(gfx::Point point_in_screen) {
  TRACE_EVENT1("views", "DragController::GetTargetTabStripForPoint",
               "point_in_screen", point_in_screen.ToString());

  gfx::NativeWindow local_window;
  const Liveness state = GetLocalProcessWindow(
      point_in_screen, current_state_ == DragState::kDraggingWindow,
      &local_window);
  if (state == Liveness::DELETED) {
    return std::tuple(Liveness::DELETED, nullptr);
  }

  if (local_window && CanAttachTo(local_window)) {
    views::Widget* widget =
        views::Widget::GetWidgetForNativeWindow(local_window);
    DragContext* destination_drag_context =
        widget ? reinterpret_cast<DragContext*>(
                     widget->GetNativeWindowProperty(DragContext::kDragContext))
               : nullptr;
    if (destination_drag_context &&
        DoesDragContextContain(destination_drag_context, point_in_screen)) {
      return std::tuple(Liveness::ALIVE, destination_drag_context);
    }
  }

  return std::tuple(Liveness::ALIVE,
                    current_state_ == DragState::kDraggingWindow
                        ? attached_context_.get()
                        : nullptr);
}

std::vector<raw_ptr<views::View, VectorExperimental>>
DragController::GetViewsMatchingDraggedContents(DragContext* context) {
  return context->GetViewsMatchingDraggedContents();
}

void DragController::MoveAttached(gfx::Point point_in_screen,
                                  bool just_attached) {
  // NOTREACHED() << __func__ << " not impl.";
  DCHECK(attached_context_);
  DCHECK(current_state_ == DragState::kDraggingTabs ||
         current_state_ == DragState::kWaitingToExitRunLoop)
      << "MoveAttached called with invalid DragState "
      << static_cast<std::underlying_type<DragState>::type>(current_state_);
}

DragController::Liveness DragController::RunMoveLoop(
    gfx::Point point_in_screen,
    gfx::Vector2d drag_offset) {
  move_loop_widget_ = GetAttachedWidget();
  DCHECK(move_loop_widget_);
  move_loop_widget_->SetBounds(
      gfx::Rect(point_in_screen - drag_offset, move_loop_widget_->GetSize()));

  current_state_ = DragState::kDraggingWindow;
  base::WeakPtr<DragController> ref(weak_factory_.GetWeakPtr());
  if (can_release_capture_) {
    // Running the move loop releases mouse capture, which triggers destroying
    // the drag loop. Release mouse capture now while the DragController is not
    // owned by the TabDragContext.
    std::unique_ptr<DragController> me =
        attached_context_->ReleaseDragController();
    DCHECK_EQ(me.get(), this);
    attached_context_->GetWidget()->ReleaseCapture();
    attached_context_->OwnDragController(std::move(me));
  }

  const views::Widget::MoveLoopSource move_loop_source =
      event_source_ == ui::mojom::DragEventSource::kMouse
          ? views::Widget::MoveLoopSource::kMouse
          : views::Widget::MoveLoopSource::kTouch;
  const views::Widget::MoveLoopEscapeBehavior escape_behavior =
      is_dragging_new_browser_
          ? views::Widget::MoveLoopEscapeBehavior::kHide
          : views::Widget::MoveLoopEscapeBehavior::kDontHide;

  // This code isn't set up to handle nested run loops. Nested run loops may
  // lead to all sorts of interesting crashes, and generally indicate a bug
  // lower in the stack. This is a CHECK() as there may be security
  // implications to attempting a nested run loop.

  CHECK(!in_move_loop_);
  in_move_loop_ = true;
  std::ignore = move_loop_widget_->RunMoveLoop(drag_offset, move_loop_source,
                                               escape_behavior);
  // Note: `this` can be deleted here!

  if (!ref) {
    return Liveness::DELETED;
  }

  in_move_loop_ = false;
  move_loop_widget_ = nullptr;

  if (current_state_ == DragState::kWaitingToExitRunLoop) {
    current_state_ = DragState::kDraggingTabs;
  } else if (current_state_ == DragState::kWaitingToDragTabs) {
    DCHECK(drag_views_to_attach_to_after_exit_);
    DetachAndAttachToNewContext(DONT_RELEASE_CAPTURE,
                                drag_views_to_attach_to_after_exit_);
    current_state_ = DragState::kDraggingTabs;
    // Move the tabs into position.
    MoveAttached(GetCursorScreenPoint(), true);
    attached_context_->GetWidget()->Activate();
    // Activate may trigger a focus loss, destroying us.
    if (!ref) {
      return Liveness::DELETED;
    }
    drag_views_to_attach_to_after_exit_ = nullptr;
  } else if (current_state_ == DragState::kDraggingWindow) {
    EndDrag();
    return Liveness::DELETED;
  }

  return Liveness::ALIVE;
}

DragController::Liveness DragController::SetCapture(DragContext* context) {
  auto ref = weak_factory_.GetWeakPtr();
  context->GetWidget()->SetCapture(context);
  return ref ? Liveness::ALIVE : Liveness::DELETED;
}

void DragController::StartDrag() {
  // `source_context_` already owns `this` (it created us, even), so no need
  // to hand off ownership.
  CHECK_EQ(source_context_->GetDragController(), this);
  attached_context_ = source_context_;

  // // Request a thumbnail to use as drag image if we'll use fallback tab
  // // dragging. Do this before calling AttachImpl() to minimize the delay
  // between
  // // detaching the tabs and showing the drag icon, as capturing the tab
  // // thumbnail is asynchronous.
  // if (ShouldDragWindowUsingSystemDnD()) {
  //   RequestTabThumbnail();
  // }

  AttachImpl();
}
