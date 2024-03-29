#pragma once

class DayNightCycle {

	
private:
	enum State {DAY = 0, NIGHT = 1};

	State m_state;
	int m_currentTurn;
	int m_currentCycle;

	void computeState() {

		m_currentCycle = m_currentTurn / 40u;
		m_state = (m_currentTurn % 40 < 30) ? DAY : NIGHT;

	}

public:

	DayNightCycle() 
		: m_state(DAY)
		, m_currentTurn(-1)
		, m_currentCycle(0)
	{}

	explicit DayNightCycle(const int currentTurn) {	updateTurn(currentTurn);	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	void updateTurn(const int currentTurn)	noexcept { m_currentTurn = currentTurn; computeState(); }
	void incrementTurn()					noexcept { updateTurn(m_currentTurn + 1);				}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// -- Getters
	
	[[nodiscard]] bool isDay()			const noexcept { return m_state == DAY;		}
	[[nodiscard]] bool isNight()		const noexcept { return m_state == NIGHT;	}
	[[nodiscard]] int getCurrentDay()	const noexcept { return m_currentTurn;		}
	[[nodiscard]] int getCurrentCycle() const noexcept { return m_currentCycle;		} // might be useful for late game strat
	[[nodiscard]] int timeBeforeNight() const noexcept { return std::max(0, 30 - (m_currentTurn % 40));	} 
	[[nodiscard]] int timeBeforeDay()	const noexcept { return std::min(10, 40 - (m_currentTurn % 40)); }


};