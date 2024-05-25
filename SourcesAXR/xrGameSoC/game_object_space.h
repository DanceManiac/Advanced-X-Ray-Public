#pragma once

namespace GameObject {
	enum ECallbackType {
		eTradeStart = u32(0),
		eTradeStop,
		eTradeSellBuyItem,
		eTradePerformTradeOperation,

		eZoneEnter,
		eZoneExit,
		eExitLevelBorder,
		eEnterLevelBorder,
		eDeath,

		ePatrolPathInPoint,

		eInventoryPda,
		eInventoryInfo,
		eArticleInfo,
		eTaskStateChange,
		eMapLocationAdded,

		eUseObject,

		eHit,

		eSound,

		eActionTypeMovement,
		eActionTypeWatch,
		eActionTypeAnimation,
		eActionTypeSound,
		eActionTypeParticle,
		eActionTypeObject,
		eActionTypeWeaponFire,

		eActorSleep,

		eHelicopterOnPoint,
		eHelicopterOnHit,

		eOnItemTake,
		eOnItemDrop,

		eScriptAnimation,
		
		eTraderGlobalAnimationRequest,
		eTraderHeadAnimationRequest,
		eTraderSoundEnd,

		eInvBoxItemTake,
		// vehicle
		eAttachVehicle,
		eDetachVehicle,
		eUseVehicle,

		eDummy = u32(-1),
	};
};