// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "GameInventorySystem.h"

#include "GISSlotBaseWidget.h"
#include "GISItemBaseWidget.h"
#include "GISTabBaseWidget.h"

#include "../GISGlobalTypes.h"
#include "../GISInventoryBaseComponent.h"

#include "GISContainerBaseWidget.h"

UGISContainerBaseWidget::UGISContainerBaseWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UGISContainerBaseWidget::InitializeContainer()
{
	if (InventoryComponent)
	{
		TArray<FGISSlotInfo> ItemInfos = InventoryComponent->GetInventoryArray();
		int32 IndexCounter = 0;

		for (const FGISTabInfo& Tab : InventoryComponent->Tabs.InventoryTabs)
		{
			if (TabClass && SlotClass)
			{
				UGISTabBaseWidget* tabWidget = ConstructObject<UGISTabBaseWidget>(TabClass, this);
				if (tabWidget)
				{
					ULocalPlayer* Player = InventoryComponent->GetWorld()->GetFirstLocalPlayerFromController(); //temporary
					tabWidget->SetPlayerContext(FLocalPlayerContext(Player)); //temporary
					tabWidget->Initialize();
					tabWidget->TabInfo = Tab;

					
					for (const FGISSlotInfo& Slot : Tab.TabSlots)
					{
						UGISSlotBaseWidget* slotWidget = ConstructObject<UGISSlotBaseWidget>(SlotClass, this);
						if (slotWidget)
						{
							ULocalPlayer* Player = InventoryComponent->GetWorld()->GetFirstLocalPlayerFromController(); //temporary
							slotWidget->SetPlayerContext(FLocalPlayerContext(Player)); //temporary
							slotWidget->Initialize();
							slotWidget->SlotInfo = Slot;
							slotWidget->GISItemClass = ItemClass;
							tabWidget->InventorySlots.Add(slotWidget);
						}
					}

					InventoryTabs.Add(tabWidget);
				}
			}
		}
		//bind functions to delegates:
		InventoryComponent->OnItemAdded.AddDynamic(this, &UGISContainerBaseWidget::Widget_OnItemAdded);
		InventoryComponent->OnItemSlotSwapped.AddDynamic(this, &UGISContainerBaseWidget::Widget_OnItemSlotSwapped);
	}
}

void UGISContainerBaseWidget::Widget_OnItemAdded(const FGISSlotUpdateData& SlotUpdateInfo)
{
	if (SlotUpdateInfo.SlotData)
	{
		UGISItemBaseWidget* ItemWidget = ConstructObject<UGISItemBaseWidget>(ItemClass, this);
		if (ItemWidget && InventoryComponent)
		{
			ULocalPlayer* Player = InventoryComponent->GetWorld()->GetFirstLocalPlayerFromController(); //temporary
			ItemWidget->SetPlayerContext(FLocalPlayerContext(Player)); //temporary
			ItemWidget->Initialize();
			//ItemWidget->LastSlotInfo = SlotInfo;
		}
		
		UWidget* superWidget = InventoryTabs[SlotUpdateInfo.TabIndex]->InventorySlots[SlotUpdateInfo.SlotIndex]->GetWidgetFromName(TEXT("OverlaySlot"));
		
		UOverlay* overlay = Cast<UOverlay>(superWidget);
		if (overlay)
		{
			int32 childCount = overlay->GetChildrenCount();
			if (childCount > 1)
			{
				overlay->RemoveChildAt(childCount - 1);
			}
			overlay->AddChild(ItemWidget);
		}
	}
}
void UGISContainerBaseWidget::Widget_OnItemSlotSwapped(const FGISSlotSwapInfo& SlotSwapInfo)
{
	if (SlotSwapInfo.LastSlotComponent == SlotSwapInfo.TargetSlotComponent)
	{
		if (!SlotSwapInfo.LastSlotData)
		{
			RemoveItem(SlotSwapInfo);
			AddItem(SlotSwapInfo);
		}
	}
	//so we targeted different component with out drop action.
	//we need to handle it! but how...
	if (SlotSwapInfo.LastSlotComponent != SlotSwapInfo.TargetSlotComponent)
	{
		if (SlotSwapInfo.LastSlotComponent.IsValid() && SlotSwapInfo.TargetSlotComponent.IsValid())
		{
			//actually, probabaly need separate functions, as there might be more
			//complex scases like actuall swapping items, instead of puting it
			//in empty slot in another component.
			SlotSwapInfo.LastSlotComponent->InventoryContainer->RemoveItem(SlotSwapInfo);
			SlotSwapInfo.TargetSlotComponent->InventoryContainer->AddItem(SlotSwapInfo);
		}
	}
	UGISContainerBaseWidget* awesomeTest = this;
	float lolo = 10;
}

void UGISContainerBaseWidget::AddItem(const FGISSlotSwapInfo& SlotSwapInfo)
{
	if (!SlotSwapInfo.LastSlotData)
	{
		UGISItemBaseWidget* ItemWidget = ConstructObject<UGISItemBaseWidget>(ItemClass, this);
		if (ItemWidget && InventoryComponent)
		{
			ULocalPlayer* Player = InventoryComponent->GetWorld()->GetFirstLocalPlayerFromController(); //temporary
			ItemWidget->SetPlayerContext(FLocalPlayerContext(Player)); //temporary
			ItemWidget->Initialize();
			//ItemWidget->LastSlotInfo = SlotInfo;
		}
		UWidget* superWidget = InventoryTabs[SlotSwapInfo.TargetTabIndex]->InventorySlots[SlotSwapInfo.TargetSlotIndex]->GetWidgetFromName(DropSlottName);

		UOverlay* overlay = Cast<UOverlay>(superWidget);
		if (overlay)
		{
			overlay->AddChild(ItemWidget);
		}
	}
	else
	{
		//construct target and last, since this is for test one will do just as well.
		UGISItemBaseWidget* ItemWidget = ConstructObject<UGISItemBaseWidget>(ItemClass, this);
		if (ItemWidget && InventoryComponent)
		{
			ULocalPlayer* Player = InventoryComponent->GetWorld()->GetFirstLocalPlayerFromController(); //temporary
			ItemWidget->SetPlayerContext(FLocalPlayerContext(Player)); //temporary
			ItemWidget->Initialize();
			//ItemWidget->LastSlotInfo = SlotInfo;
		}
		UWidget* lastSlotWidget = InventoryTabs[SlotSwapInfo.LastTabIndex]->InventorySlots[SlotSwapInfo.LastSlotIndex]->GetWidgetFromName(DropSlottName);
		UWidget* targetSlotWidget = InventoryTabs[SlotSwapInfo.TargetTabIndex]->InventorySlots[SlotSwapInfo.TargetSlotIndex]->GetWidgetFromName(DropSlottName);
		
		UOverlay* lastSlotOverlay = Cast<UOverlay>(lastSlotWidget);
		if (lastSlotOverlay)
		{
			lastSlotOverlay->AddChild(ItemWidget);
		}

		UOverlay* targetSlotOverlay = Cast<UOverlay>(targetSlotWidget);
		if (targetSlotOverlay)
		{
			targetSlotOverlay->AddChild(ItemWidget);
		}
	}
}
void UGISContainerBaseWidget::RemoveItem(const FGISSlotSwapInfo& SlotSwapInfo)
{
	if (!SlotSwapInfo.LastSlotData)
	{
		UWidget* superWidget = InventoryTabs[SlotSwapInfo.LastTabIndex]->InventorySlots[SlotSwapInfo.LastSlotIndex]->GetWidgetFromName(DropSlottName);

		//this it bit fiddly since the widget which will contain our widget must be last
		//and out item widget must be last child within this widget
		UOverlay* overlay = Cast<UOverlay>(superWidget);
		if (overlay)
		{
			int32 childCount = overlay->GetChildrenCount();
			if (childCount > 1)
			{
				overlay->RemoveChildAt(childCount - 1);
			}
		}
	}
	else
	{
		UWidget* lastSlotWidget = InventoryTabs[SlotSwapInfo.LastTabIndex]->InventorySlots[SlotSwapInfo.LastSlotIndex]->GetWidgetFromName(DropSlottName);
		UWidget* targetSlotWidget = InventoryTabs[SlotSwapInfo.TargetTabIndex]->InventorySlots[SlotSwapInfo.TargetSlotIndex]->GetWidgetFromName(DropSlottName);
		//this it bit fiddly since the widget which will contain our widget must be last
		//and out item widget must be last child within this widget
		UOverlay* lastSlotOverlay = Cast<UOverlay>(lastSlotWidget);
		if (lastSlotOverlay)
		{
			int32 childCount = lastSlotOverlay->GetChildrenCount();
			if (childCount > 1)
			{
				lastSlotOverlay->RemoveChildAt(childCount - 1);
			}
		}
		UOverlay* targetSlotOverlay = Cast<UOverlay>(targetSlotWidget);
		if (lastSlotOverlay)
		{
			int32 childCount = targetSlotOverlay->GetChildrenCount();
			if (childCount > 1)
			{
				targetSlotOverlay->RemoveChildAt(childCount - 1);
			}
		}
	}
}