// Copyright 2025 The Radium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIUM_BROWSER_UI_VIEWS_DRAGGING_DRAG_CONTEXT_H_
#define RADIUM_BROWSER_UI_VIEWS_DRAGGING_DRAG_CONTEXT_H_

#include "ui/gfx/geometry/vector2d.h"
#include "ui/views/view.h"

class DragController;

class DragContext : public views::View {
  METADATA_HEADER(DragContext, views::View)

 public:
  static constexpr const char kDragContext[] = "__DRAG_CONTEXT__";

  explicit DragContext();
  DragContext(const DragContext&) = delete;
  DragContext& operator=(const DragContext&) = delete;

  ~DragContext() override;

  // Returns true if a drag session is currently active.
  virtual bool IsDragSessionActive() const = 0;

  // Returns true if this DragContext is in the process of returning tabs to the
  // associated TabContainer.
  virtual bool IsAnimatingDragEnd() const = 0;

  // Immediately completes any ongoing end drag animations, returning the tabs
  // to the associated TabContainer immediately.
  virtual void CompleteEndDragAnimations() = 0;

  // Destroys the current TabDragController. This cancel the existing drag
  // operation.
  virtual void DestroyDragController() = 0;

  // Returns the tab drag controller owned by this delegate, or null if none.
  virtual DragController* GetDragController() = 0;

  // Returns the insets of the draggable area. This allows the area where the
  // dragging actually occurs to be larger or smaller than the size of the
  // TabContext itself.
  virtual gfx::Insets GetDragAreaInsets() const = 0;

  virtual gfx::Size GetDraggedWindowSize() const = 0;

  // Called when the drag context cannot be found.
  virtual DragContext* CreateNewDragContext() = 0;

  // Invoked when DragController detaches a set of views.
  virtual void DragContextDetached() = 0;

  // Remove the view from the original DragContext and transfer ownership.
  virtual std::unique_ptr<views::View> DetachView(views::View* view) = 0;

  // Remove the view from the original DragContext and transfer ownership.
  virtual void AttachView(std::unique_ptr<views::View> view) = 0;

  // Returns the possible offset of the mouse position from the target window.
  virtual const views::View* GetAnchorView() const = 0;

  // Get all the views that are currently being dragged.
  virtual std::vector<raw_ptr<views::View, VectorExperimental>>
  GetViewsMatchingDraggedContents() = 0;

  // Takes ownership of `controller`.
  virtual void OwnDragController(
      std::unique_ptr<DragController> controller) = 0;

  // Releases ownership of the current TabDragController.
  [[nodiscard]] virtual std::unique_ptr<DragController>
  ReleaseDragController() = 0;

  // Used by DragController when the user starts or stops dragging.
  virtual void StartedDragging(
      const std::vector<raw_ptr<views::View, VectorExperimental>>& views) = 0;

  virtual bool CanAttachTo(gfx::NativeWindow window) = 0;

  // views::View:
  void AddedToWidget() override;
};

#endif  // RADIUM_BROWSER_UI_VIEWS_DRAGGING_DRAG_CONTEXT_H_
