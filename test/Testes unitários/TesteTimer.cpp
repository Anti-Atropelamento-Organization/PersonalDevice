#include <Arduino.h>

// --- COLANDO A CLASSE AQUI PARA FACILITAR O TESTE EM UM ARQUIVO SÓ ---
class SimpleTimer {
  private:
    unsigned long _lastTime;
    unsigned long _interval;

  public:
    SimpleTimer(unsigned long intervalMs) {
        _interval = intervalMs;
        _lastTime = millis();
    }
    bool isReady() {
        return (millis() - _lastTime >= _interval);
    }
    void reset() {
        _lastTime = millis();
    }
    void setInterval(unsigned long intervalMs) {
        _interval = intervalMs;
    }
    unsigned long getInterval() {
        return _interval;
    }
};
// ---------------------------------------------------------------------

// Instanciando dois timers para teste
SimpleTimer timerFixo(2000);    // Vai disparar a cada 2 segundos sempre
SimpleTimer timerDinamico(5000); // Começa com 5 segundos, mas vai mudar

void setup() {
    Serial.begin(9600);
    randomSeed(analogRead(0));
    
    Serial.println("--- INICIANDO TESTE DE TEMPORIZACAO ---");
    Serial.println("Timer Fixo: 2000ms");
    Serial.println("Timer Dinamico: 5000ms (com simulacao de erro)");
    Serial.println("-----------------------------------------");
}

void loop() {
    
    // CASO 1: Timer simples e repetitivo
    if (timerFixo.isReady()) {
        Serial.print("[FIXO] 2 segundos se passaram. Time: ");
        Serial.println(millis());
        
        timerFixo.reset(); // Reinicia para contar mais 2 segundos
    }

    // CASO 2: Timer com lógica de "Backoff" (Simula seu caso do LoRa)
    if (timerDinamico.isReady()) {
        Serial.print("[DINAMICO] Disparou! Intervalo atual era: ");
        Serial.print(timerDinamico.getInterval());
        Serial.println(" ms");

        // Simulação: Se o intervalo era 5000, finge que o canal estava ocupado
        if (timerDinamico.getInterval() == 5000) {
            long tempoAleatorio = random(500, 1000);
            
            Serial.print("   -> Simulando CANAL OCUPADO. Tentando novamente em ");
            Serial.print(tempoAleatorio);
            Serial.println(" ms...");
            
            timerDinamico.setInterval(tempoAleatorio); // Muda para tempo curto
        } 
        // Se o intervalo era curto (significa que foi uma re-tentativa), volta ao normal
        else {
            Serial.println("   -> Re-tentativa (Backoff) executada com sucesso!");
            Serial.println("   -> Voltando para 5000 ms.");
            
            timerDinamico.setInterval(5000); // Volta para o tempo longo
        }

        timerDinamico.reset(); // Aplica a mudança e reinicia a contagem
    }
}