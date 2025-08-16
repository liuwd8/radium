// Copyright 2025 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_DRAGGING_DRAG_CONTROLLER_H_
#define RADIUM_BROWSER_UI_VIEWS_DRAGGING_DRAG_CONTROLLER_H_

#include "base/scoped_observation.h"
#include "base/timer/timer.h"
#include "ui/base/dragdrop/mojom/drag_drop_types.mojom-shared.h"
#include "ui/gfx/geometry/point.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/views/widget/widget_observer.h"

class DragContext;
class WindowFinder;

namespace views {
class View;
class Widget;
class ViewTracker;
}  // namespace views

class DragController : public views::WidgetObserver {
 public:
  explicit DragController();
  DragController(const DragController&) = delete;
  DragController& operator=(const DragController&) = delete;

  ~DragController() override;

  enum class Liveness {
    ALIVE,
    DELETED,
  };

  // Whether Detach() should release capture or not.
  enum ReleaseCapture {
    RELEASE_CAPTURE,
    DONT_RELEASE_CAPTURE,
  };

  // Enum passed to EndDrag().
  enum EndDragReason {
    // Complete the drag.
    END_DRAG_COMPLETE,

    // Cancel/revert the drag.
    END_DRAG_CANCEL,

    // The drag should end as the result of a capture lost.
    END_DRAG_CAPTURE_LOST,

    // The model mutated.
    END_DRAG_MODEL_ADDED_TAB,
  };

  ui::mojom::DragEventSource event_source() const { return event_source_; }

  // See description above fields for details on these.
  bool active() const { return current_state_ != DragState::kStopped; }
  const DragContext* attached_context() const { return attached_context_; }

  // Returns true if a drag started.
  bool started_drag() const { return current_state_ != DragState::kNotStarted; }

  // Initializes TabDragController to drag the views in `dragging_views`
  // originating from `source_context`. `source_view` is the view that
  // initiated the drag and is either a Tab or a TabGroupHeader contained in
  // `dragging_views`. `mouse_offset` is the distance of the mouse pointer from
  // the origin of the first view in `dragging_views` and `source_view_offset`
  // the offset from `source_view`. `source_view_offset` is the horizontal
  // offset of `mouse_offset` relative to `source_view`.
  // `initial_selection_model` is the selection model before the drag started
  // and is only non-empty if the original selection isn't the same as the
  // dragging set. Returns Liveness::DELETED if `this` was deleted during this
  // call, and Liveness::ALIVE if `this` still exists.
  [[nodiscard]] Liveness Init(DragContext* source_context,
                              views::View* source_view,
                              const gfx::Point& mouse_offset,
                              int source_view_offset,
                              ui::mojom::DragEventSource event_source);

  // Invoked to drag to the new location, in screen coordinates.
  [[nodiscard]] Liveness Drag(const gfx::Point& point_in_screen);

  // Complete the current drag session.
  void EndDrag(EndDragReason reason);

 private:
  enum class DragState {
    // The drag has not yet started; the user has not dragged far enough to
    // begin a session.
    kNotStarted,
    // The session is dragging a set of tabs within `attached_context_`.
    kDraggingTabs,
    // The session is dragging a window; `attached_context_` is that window's
    // tabstrip.
    kDraggingWindow,
    // The platform does not support client controlled window dragging; instead,
    // a regular drag and drop session is running. The dragged tabs are still
    // moved to a new browser, but it stays hidden until the drag ends. On
    // platforms where this state is used, the kDraggingWindow and
    // kWaitingToDragTabs states are not used.
    kDraggingUsingSystemDnD,
    // The session has already attached to the target tabstrip, but must wait
    // for the nested move loop to exit to transition to kDraggingTabs. Used on
    // platforms where `can_release_capture_` is false.
    kWaitingToExitRunLoop,
    // The session is still attached to the drag-created window, and is waiting
    // for the nested move loop to exit to transition to kDraggingTabs and
    // attach to `tab_strip_to_attach_to_after_exit_`. Used on platforms where
    // `can_release_capture_` is true.
    kWaitingToDragTabs,
    // The drag session has completed or been canceled.
    kStopped
  };

  // views::WidgetObserver:
  void OnWidgetBoundsChanged(views::Widget* widget,
                             const gfx::Rect& new_bounds) override;
  void OnWidgetDestroyed(views::Widget* widget) override;

  // Forget the source tabstrip. It doesn't exist any more, so it doesn't
  // make sense to insert dragged tabs back into it if the drag is reverted.
  void OnSourceContextDestory();

  // Sets up dragging in `attached_context_`. The dragged tabs must already
  // be present.
  void AttachImpl();

  // Insert the dragged tabs into `attached_context` and attach the drag session
  // to it. The newly attached context will have capture, and will take
  // ownership of `controller` (which must be `this`).
  void AttachToNewContext(DragContext* attached_context,
                          std::unique_ptr<DragController> controller,
                          std::vector<std::unique_ptr<views::View>> owned_tabs);

  // Tests whether a drag can be attached to a `window`.  Drags may be
  // disallowed for reasons such as the target: does not support tabs, is
  // showing a modal, has a different profile, is a different browser type
  // (NORMAL vs APP).
  bool CanAttachTo(gfx::NativeWindow window);

  // Saves focus in the window that the drag initiated from. Focus will be
  // restored appropriately if the drag ends within this same window.
  [[nodiscard]] Liveness SaveFocus();

  // Restore focus to the View that had focus before the drag was started, if
  // the drag ends within the same Window as it began.
  void RestoreFocus();

  // Tests whether `point_in_screen` is past a minimum elasticity threshold
  // required to start a drag.
  bool CanStartDrag(const gfx::Point& point_in_screen) const;

  // Invoked once a drag has started to determine the appropriate context to
  // drag to (which may be the currently attached one).
  [[nodiscard]] Liveness ContinueDragging(const gfx::Point& point_in_screen);

  // Detach the dragged tabs from the current TabDragContext. Returns
  // ownership of the owned controller, which must be `this`, if
  // `attached_context_` currently owns a controller. Otherwise returns
  // nullptr.
  std::tuple<std::unique_ptr<DragController>,
             std::vector<std::unique_ptr<views::View>>>
  Detach(ReleaseCapture release_capture);

  // Detach from `attached_context_` and attach to `target_context` instead.
  // See Detach/Attach for parameter documentation. Transfers ownership of
  // `this` from `attached_context_` (which must own `this`) to
  // `target_context`.
  void DetachAndAttachToNewContext(ReleaseCapture release_capture,
                                   DragContext* target_context);

  // Detaches the tabs being dragged, creates a new Browser to contain them and
  // runs a nested move loop.
  [[nodiscard]] Liveness DetachIntoNewWidgetAndRunMoveLoop(
      gfx::Point point_in_screen);

  // Returns true if `context` contains the specified point in screen
  // coordinates.
  bool DoesDragContextContain(DragContext* context,
                              const gfx::Point& point_in_screen) const;

  // Transitions dragging from `attached_context_` to `target_context`.
  // `target_context` is nullptr if the mouse is not over a valid tab strip.
  [[nodiscard]] Liveness DragContainerToNewWidget(
      DragContext* target_context,
      const gfx::Point& point_in_screen);

  // Returns the Widget of the currently attached TabDragContext's
  // BrowserView.
  views::Widget* GetAttachedWidget();

  // Returns the location of the cursor. This is either the location of the
  // mouse or the location of the current touch point.
  gfx::Point GetCursorScreenPoint();

  // Calculates the drag offset needed to place the correct point on the
  // source view under the cursor.
  gfx::Vector2d CalculateWindowDragOffset();

  // Returns the NativeWindow in `window` at the specified point. If
  // `exclude_dragged_view` is true, then the dragged view is not considered.
  [[nodiscard]] Liveness GetLocalProcessWindow(const gfx::Point& screen_point,
                                               bool exclude_dragged_view,
                                               gfx::NativeWindow* window);

  // Returns the compatible TabDragContext to drag to at the specified point
  // (screen coordinates), or nullptr if there is none. May end the drag on
  // some platforms as a result of reentrancy during system calls, hence this
  // also returns a Liveness.
  [[nodiscard]] std::tuple<Liveness, DragContext*> GetTargetDragContextForPoint(
      gfx::Point point_in_screen);

  // Finds the TabSlotViews within the specified TabDragContext that
  // corresponds to the WebContents of the dragged views. Also finds the group
  // header if it is dragging. Returns an empty vector if not attached.
  std::vector<raw_ptr<views::View, VectorExperimental>>
  GetViewsMatchingDraggedContents(DragContext* context);

  void MoveAttached(gfx::Point point_in_screen, bool just_attached);

  // Runs a nested run loop that handles moving the current Browser.
  // `drag_offset` is the desired offset between the cursor and the window
  // origin. `point_in_screen` is the cursor location in screen space.
  [[nodiscard]] Liveness RunMoveLoop(gfx::Point point_in_screen,
                                     gfx::Vector2d drag_offset);

  void BringWindowUnderPointToFront(const gfx::Point& point_in_screen);

  [[nodiscard]] Liveness SetCapture(DragContext* context);

  // Begin the drag session by attaching to `source_context_`.
  void StartDrag();

  DragState current_state_ = DragState::kNotStarted;

  ui::mojom::DragEventSource event_source_ = ui::mojom::DragEventSource::kMouse;

  // The TabDragContext to attach to after the move loop completes.
  raw_ptr<DragContext> drag_views_to_attach_to_after_exit_;

  // Non-null for the duration of RunMoveLoop.
  raw_ptr<views::Widget> move_loop_widget_;

  std::unique_ptr<WindowFinder> window_finder_;

  base::ScopedObservation<views::Widget, views::WidgetObserver>
      widget_observation_{this};

  // The DragContext the drag originated from. This is set to null
  // if destroyed during the drag.
  raw_ptr<DragContext> source_context_;

  std::unique_ptr<views::ViewTracker> source_context_destory_tracker_;

  // The DragContext the dragged Tab is currently attached to, or
  // null if the dragged Tab is detached.
  raw_ptr<DragContext> attached_context_;

  // Whether capture can be released during the drag. When false, capture should
  // not be released when transferring capture between widgets and when starting
  // the move loop.
  bool can_release_capture_ = true;

  // The position of the mouse (in screen coordinates) at the start of the drag
  // operation. This is used to calculate minimum elasticity before a
  // DraggedTabView is constructed.
  gfx::Point start_point_in_screen_;

  // This is the offset of the mouse from the top left of the first Tab where
  // dragging began. This is used to ensure that the dragged view is always
  // positioned at the correct location during the drag, and to ensure that the
  // detached window is created at the right location.
  gfx::Point mouse_offset_;

  // Used to track the view that had focus in the window containing
  // `source_view_`. This is saved so that focus can be restored properly when
  // a drag begins and ends within this same window.
  std::unique_ptr<views::ViewTracker> old_focused_view_tracker_;

  // Timer used to bring the window under the cursor to front. If the user
  // stops moving the mouse for a brief time over a browser window, it is
  // brought to front.
  base::OneShotTimer bring_to_front_timer_;

  // True while RunMoveLoop() has been called on a widget.
  bool in_move_loop_ = false;

  // True if `attached_context_` is in a browser specifically created for
  // the drag.
  bool is_dragging_new_browser_;

  base::WeakPtrFactory<DragController> weak_factory_{this};
};

#endif  // RADIUM_BROWSER_UI_VIEWS_DRAGGING_DRAG_CONTROLLER_H_
