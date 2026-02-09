#ifndef SIMPLETIMER_H
#define SIMPLETIMER_H

#include <Arduino.h> 

class SimpleTimer {
  private:
    unsigned long _lastTime;
    unsigned long _interval;

  public:
    // Construtor: Inicializa com um intervalo em ms
    SimpleTimer(unsigned long intervalMs);

    // Verifica se o tempo já estourou
    bool isReady();

    // Reinicia a contagem do tempo a partir de agora
    void reset();

    // Altera o intervalo da temporização
    void setInterval(unsigned long intervalMs);

    // Retorna o valor atual do intervalo configurado
    unsigned long getInterval();
};

#endif