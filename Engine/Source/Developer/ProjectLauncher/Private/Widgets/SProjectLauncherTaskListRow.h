// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once


#define LOCTEXT_NAMESPACE "SProjectLauncherTaskListRow"


/**
 * Implements a row widget for the launcher's task list.
 */
class SProjectLauncherTaskListRow
	: public SMultiColumnTableRow<ILauncherTaskPtr>
{
public:

	SLATE_BEGIN_ARGS(SProjectLauncherTaskListRow) { }
		SLATE_ARGUMENT(TSharedPtr<STableViewBase>, OwnerTableView)
		SLATE_ARGUMENT(ILauncherTaskPtr, Task)
	SLATE_END_ARGS()

public:

	/**
	 * Constructs the widget.
	 *
	 * @param InArgs The construction arguments.
	 * @param InDeviceProxyManager The device proxy manager to use.
	 */
	void Construct( const FArguments& InArgs )
	{
		Task = InArgs._Task;

		SMultiColumnTableRow<ILauncherTaskPtr>::Construct(FSuperRowType::FArguments(), InArgs._OwnerTableView.ToSharedRef());
	}

public:

	/**
	 * Generates the widget for the specified column.
	 *
	 * @param ColumnName The name of the column to generate the widget for.
	 * @return The widget.
	 */
	virtual TSharedRef<SWidget> GenerateWidgetForColumn( const FName& ColumnName ) override;

private:

	// Callback for getting the duration of the task.
	FString HandleDurationText( ) const
	{
		ILauncherTaskPtr TaskPtr = Task.Pin();

		if (TaskPtr.IsValid())
		{
			if ((TaskPtr->GetStatus() != ELauncherTaskStatus::Pending) &&
				(TaskPtr->GetStatus() != ELauncherTaskStatus::Canceled))
			{
				return TaskPtr->GetDuration().ToString(TEXT("%h:%m:%s"));
			}
		}

		return FString();
	}

	// Callback for getting the color and opacity of the status icon.
	FSlateColor HandleIconColorAndOpacity( ) const
	{
		ILauncherTaskPtr TaskPtr = Task.Pin();

		if (TaskPtr.IsValid())
		{
			if (TaskPtr->GetStatus() == ELauncherTaskStatus::Canceled)
			{
				return FLinearColor::Yellow;
			}

			if (TaskPtr->GetStatus() == ELauncherTaskStatus::Completed)
			{
				return FLinearColor::Green;
			}

			if (TaskPtr->GetStatus() == ELauncherTaskStatus::Failed)
			{
				return FLinearColor::Red;
			}
		}

		return FSlateColor::UseForeground();
	}

	// Callback for getting the status icon image.
	const FSlateBrush* HandleIconImage( ) const
	{
		ILauncherTaskPtr TaskPtr = Task.Pin();

		if (TaskPtr.IsValid())
		{
			if (TaskPtr->GetStatus() == ELauncherTaskStatus::Canceled)
			{
				return FEditorStyle::GetBrush("Icons.Cross");
			}

			if (TaskPtr->GetStatus() == ELauncherTaskStatus::Completed)
			{
				return FEditorStyle::GetBrush("Symbols.Check");
			}

			if (TaskPtr->GetStatus() == ELauncherTaskStatus::Failed)
			{
				return FEditorStyle::GetBrush("Icons.Cross");
			}
		}

		return NULL;
	}

	// Callback for getting the task's status text.
	FString HandleStatusText( ) const
	{
		ILauncherTaskPtr TaskPtr = Task.Pin();

		if (TaskPtr.IsValid())
		{
			ELauncherTaskStatus::Type TaskStatus = TaskPtr->GetStatus();

			switch (TaskStatus)
			{
			case ELauncherTaskStatus::Busy:

				return LOCTEXT("StatusInProgressText", "Busy").ToString();

			case ELauncherTaskStatus::Canceled:

				return LOCTEXT("StatusCanceledText", "Canceled").ToString();

			case ELauncherTaskStatus::Canceling:

				return LOCTEXT("StatusCancelingText", "Canceling").ToString();

			case ELauncherTaskStatus::Completed:

				return LOCTEXT("StatusCompletedText", "Completed").ToString();

			case ELauncherTaskStatus::Failed:

				return LOCTEXT("StatusFailedText", "Failed").ToString();

			case ELauncherTaskStatus::Pending:

				return LOCTEXT("StatusPendingText", "Pending").ToString();
			}
		}

		return FString();
	}

	// Callback for determining the throbber's visibility.
	EVisibility HandleThrobberVisibility( ) const
	{
		ILauncherTaskPtr TaskPtr = Task.Pin();

		if (TaskPtr.IsValid())
		{
			if ((TaskPtr->GetStatus() == ELauncherTaskStatus::Busy) ||
				(TaskPtr->GetStatus() == ELauncherTaskStatus::Canceling))
			{
				return EVisibility::Visible;
			}
		}

		return EVisibility::Hidden;
	}

private:

	// Holds a pointer to the task that is displayed in this row.
	TWeakPtr<ILauncherTask> Task;
};


#undef LOCTEXT_NAMESPACE
