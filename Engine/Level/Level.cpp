#include"Level.h"
#include"Actor/Actor.h"

namespace Mint
{
	Level::Level()
	{
	}

	Level::~Level()
	{
		// 메모리 정리하기
		for (Actor*& actor : actors)
		{
			// Actor 객체의 메모리를 해제한다
			if (actor)
			{
				delete actor;
				actor = nullptr;
			}
		}

		// 배열 초기화하기
		actors.clear();
	}

	void Level::BeginPlay()
	{
		// Actor에 Event를 흘리기
		for (Actor* actor : actors)
		{
			// 이미 Beginplay가 호출된 Actor의 경우에는 건너뛰어야함
			if (actor->HasBeganPlay())
			{
				continue;
			}
			actor->BeginPlay();
		}
	}
	void Level::Tick(float deltaTime)
	{
		// Actor에 이벤트를 흘린다
		for (Actor* actor : actors)
		{
			actor->Tick(deltaTime);
		}

	}
	void Level::Draw()
	{
		// Actor에 이벤트를 흘린다
		for (Actor* actor : actors)
		{
			// 같은 위치에 다른 액터가 있는지 확인을 한다
			Actor* search = nullptr;
			for (Actor* otherActor : actors)
			{
				// 같은 액터는 비교하지 않는다
				if (actor == otherActor)
				{
					continue;
				}

				// 위치 비교
				if (actor->GetPosition() == otherActor->GetPosition())
				{
					// 정렬 순서 비교
					if (actor->GetSortingOrder()
						< otherActor->GetSortingOrder())
					{
						search = otherActor;
						break;
					}
				}
			}
			
			// 같은 위치에 우선 순위가 높은 액터가 있다면 그리지 않는다
			if (search)
			{
				continue;
			}

			actor->Draw();
		}

		// 액터를 순회하며 Draw 함수 호출하기
		for (Actor* const actor : actors)
		{
			if (!actor->IsActive())
			{
				continue;
			}

			actor->Draw();
		}
	}

	void Level::AddNewActor(Actor* newActor)
	{
		// 프레임 처리(고정)를 고려하여 나중에 따로 추가적인 작업을 해야함
		// push_back은 매개변수를 l-value로서 처리를 한다
		// actors.push_back(newActor);
		addRequestedActors.emplace_back(newActor);

		// 오너쉽 설정
		newActor->SetOwner(this);
	}

	void Level::ProcessAddAndDestroyActors()
	{
		// 제거 처리
		for (int ix = 0; ix < static_cast<int>(actors.size()); )
		{
			// 제거 요청된 액터가 있는지 확인한다
			if (actors[ix]->DestroyRequested())
			{
				// 제거 요청을 하였으므로 제거를 한다
				delete actors[ix];
				actors.erase(actors.begin() + ix);			// 배열에 있는 해당 요소를 없앤다(하나씩)
				continue;
			}
			++ix;							// 인덱스 계산에 주의!!
		}

		// 추가 처리
		if (addRequestedActors.size() == 0)
		{
			return;
		}

		for (Actor* const actor : addRequestedActors)
		{
			actors.emplace_back(actor);
		}

		// 처리가 끝났으면 배열을 초기화한다
		// 0으로 만들어버림..
		addRequestedActors.clear();
	}
}