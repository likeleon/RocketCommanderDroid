#include "SpaceCamera.hpp"
#include "Game.hpp"
#include "Player.hpp"

using namespace Ogre;

namespace rcd
{
	SpaceCamera::SpaceCamera(Game &game, const Vector3 &cameraPos)
	: m_camera(NULL), m_cameraMode(CameraMode_FreeCamera), m_game(game),
	  m_yawRotation(Math::PI), m_pitchRotation(0.0f), m_pos(cameraPos),
	  m_mouseSensibility(1.0f)
	{
		m_camera = m_game.GetSceneManager().createCamera("Camera");
		m_camera->lookAt(0, 0, 0);

		// Assign mouse sensibility
		float controllerSensibility = 1.0f;
		m_mouseSensibility = 2.5f - 2.0f * controllerSensibility;
		m_mouseSensibility = std::max(0.5f, m_mouseSensibility);
	}

	SpaceCamera::~SpaceCamera()
	{
		if (m_camera)
		{
			m_game.GetSceneManager().destroyCamera(m_camera);
			m_camera = NULL;
		}
	}

	Camera& SpaceCamera::GetCamera()
	{
		assert(m_camera);
		return *m_camera;
	}

	const Camera& SpaceCamera::GetCamera() const
	{
		assert(m_camera);
		return *m_camera;
	}

	void SpaceCamera::SetInGame(bool inGame)
	{
		if (inGame)
			m_cameraMode = CameraMode_InGame;
		else
			m_cameraMode = CameraMode_MenuMode;
	}

	void SpaceCamera::Update(double timeSinceLastFrame)
	{
		if (m_cameraMode == CameraMode_FreeCamera)
			HandleFreeCamera();
		else if (m_cameraMode == CameraMode_InGame)
			HandlePlayerInput();
		else // for menu
			RandomlyRotateAround();

		UpdateCamera();
	}

	void SpaceCamera::RandomlyRotateAround()
	{
		const float moveFactor = m_game.GetMoveFactorPerSecond() * 27.5f;
		const float rotationFactor = m_game.GetMoveFactorPerSecond() * 0.125f;

		// Rotate slightly around
		Rotate(RotationAxis_Yaw,
			(0.4f + 0.25f * Math::Sin(m_game.GetTotalTimeMs() / 15040)) * rotationFactor);
		Rotate(RotationAxis_Pitch,
			(0.35f + 0.212f * Math::Cos(m_game.GetTotalTimeMs() / 38040)) * rotationFactor);

		// Just move forward
		Translate(-moveFactor, MoveDirection_Z);
	}

	void SpaceCamera::Rotate(RotationAxis axis, float angle)
	{
		if (axis == RotationAxis_Yaw)
			m_yawRotation -= angle;
		else
			m_pitchRotation -= angle;
	}

	void SpaceCamera::Translate(float amount, MoveDirections direction)
	{
		Vector3 dir = Vector3::ZERO;
		if (direction == MoveDirection_X)
			dir = -GetCamera().getRight();
		else if (direction == MoveDirection_Y)
			dir = GetCamera().getUp();
		else
			dir = GetCamera().getDirection();
		m_pos += dir * amount;
	}

	void SpaceCamera::UpdateCamera()
	{
		// Limit pitchRotation to -90 degrees to +90 degrees.
		// This will stop the player if he flys to straight up or down
		// and make it impossible to fly bottom up (which is very hard).
		if (m_pitchRotation < -Math::PI / 2.0f)
			m_pitchRotation = -Math::PI / 2.0f;
		if (m_pitchRotation > Math::PI / 2.0f)
			m_pitchRotation = Math::PI / 2.0f;

		GetCamera().setOrientation(Quaternion(Radian(-m_yawRotation), Vector3::UNIT_Y) *
			Quaternion(Radian(-m_pitchRotation), Vector3::UNIT_X));
		GetCamera().setPosition(-m_pos);
	}

	void SpaceCamera::HandlePlayerInput()
	{
		Player &player = m_game.GetPlayer();

		if (player.GetLifeTimeMs() < Player::LifeTimeZoomAndAccelerateMs)
		{
			const float speedPercentage = player.GetLifeTimeMs() / (float)Player::LifeTimeZoomAndAccelerateMs;

			player.SetStartingSpeed(speedPercentage * speedPercentage);

			// Aways move forward
			Translate(-player.GetSpeed() *
				m_game.GetMoveFactorPerSecond() *
				player.GetMovementSpeedPerSecond(), MoveDirection_Z);

			if (player.GetGameTimeMs() < 100)
			{
				m_yawRotation = Math::PI;
				m_pitchRotation = 0;
				m_pos = Vector3::ZERO;
			}
		}

		if (player.CanControlRocket())
		{
			// Consume some fuel
			player.SetFuel(player.GetFuel() - m_game.GetMoveFactorPerSecond() / Player::FuelRefillTime);

			float speedScoreFactor = 0.75f * player.GetSpeed();
			if (player.GetSpeedItemTimeout() > 0)
				speedScoreFactor *= 2.0f;

			// Increase score
			if (m_game.GetTotalFrames() % 10 == 0)
				player.SetScore(player.GetScore() + (int)(speedScoreFactor * m_game.GetElapsedTimeThisFrameInMs()));
		}

		// Don't allow any movement if still counting down or exploded
		const float rotationFactor = Player::RotationSpeedPerMouseMovement;
		if (player.CanControlRocket() == false)
		{
			if (player.IsGameOver())
			{
				// Just rotate around the rocket.
				m_yawRotation = m_game.GetTotalTimeMs() / 3592.0f;
			}
			else
			{
				// TODO
			}

			return;
		}

		//
		// Mouse/keyboard support
		//
		const float maxMoveFactor = m_game.GetMoveFactorPerSecond() * player.GetMovementSpeedPerSecond();
		const float maxSlideFactor = maxMoveFactor * Player::SlideFactor;

		// TODO: Change camera rotation when mouse is moved

		bool consumedAdditionalFuel = false;

		// TODO: Keyboard W/D

		if (player.GetSpeedItemTimeout() > 0)
		{
			player.SetSpeedItemTimeout(player.GetSpeedItemTimeout() - m_game.GetElapsedTimeThisFrameInMs());
			if (player.GetSpeedItemTimeout() < 0)
			{
				player.SetSpeedItemTimeout(0);
				if (player.GetSpeed() > Player::MaxSpeedWithoutItem)
					player.SetSpeed(Player::MaxSpeedWithoutItem);
			}
		}

		// Adjust current speed by the current player speed.
		const float moveFactor = player.GetSpeed() * maxMoveFactor;
		const float slideFactor = maxSlideFactor;

		// Always move forward
		Translate(-moveFactor, MoveDirection_Z);

		// Slide & Up/down
		likeleon::SensorHandler& sensorHandler = m_game.GetSensorHandler();
		Translate(sensorHandler.GetHorizontal() * -slideFactor, MoveDirection_X);
		Translate(sensorHandler.GetVertical() * -slideFactor, MoveDirection_Y);

		if (consumedAdditionalFuel)
		{
			player.SetFuel(player.GetFuel() - (m_game.GetMoveFactorPerSecond() / Player::FuelRefillTime) / 2.0f);
			if (m_game.GetTotalFrames() % 20 == 0)
				player.SetScore(player.GetScore() + (int)m_game.GetElapsedTimeThisFrameInMs() / 2);
		}
	}

	bool SpaceCamera::IsInGame() const
	{
		return (m_cameraMode == CameraMode_InGame);
	}
}
