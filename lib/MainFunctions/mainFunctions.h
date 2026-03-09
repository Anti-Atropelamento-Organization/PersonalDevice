#ifndef MAINFUNCTIONS_H
#define MAINFUNCTIONS_H

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "PersonalDevice.h"
#include "packet.h"
#include "SimpleTimer.h"
#include "DeviceBase.h"

/**
 * @brief Classe utilitária que centraliza as funções do dispositivo
 */
class mainFunctions {
public:

    /** @brief Envia alertas para veículos*/
    void SendAlertDest();

    /** * @brief Atualiza constantes e estados do dispositivo pessoal
     * Alimenta o GPS, atualiza HDOP, latitude, longitude e salva a posição no histórico
     * @param personal Referência ao objeto do dispositivo pessoal
     */
    void SetPersonalConst(PersonalDevice& personal);

    /**
     * @brief Gerencia a recepção de pacotes e a lógica de ACK
     * Identifica se o pacote é de segurança ou um ACK 
     * (para confirmar recebimento de logs ou monitoramento).
     * @param device Referência ao objeto
     * @param st Timer para controle do período de recebimento de pacotes
     * @param jitterTargetTime Variável para armazenar o tempo de envio com atraso aleatório
     * @param waitingToSend Flag que indica espera para envio
     * @param hasTarget Flag que indica se o pacote recebido é valido
     * @param ackMonitoring Flag de confirmação do pacote de monitoramento
     * @param ackLog Flag de confirmação do pacote de log.
     */
    void ReceivePacketDevice(DeviceBase& device, SimpleTimer& st, unsigned long& jitterTargetTime, bool& waitingToSend, bool& hasTarget, bool& ackMonitoring, bool& ackLog);

    /**
     * @brief Realiza o envio do pacote de segurança (SAFETY)
     * Verifica se o canal está livre e se há sinal GPS antes de transmitir via LoRa
     * @param device Dispositivo que realizará o envio
     * @param st_safety Timer de controle do intervalo de envio de safety
     * @param jitterTargetTime Tempo alvo calculado com jitter para evitar colisões
     */
    void SendPacketSafety(DeviceBase& device, SimpleTimer& st_safety, unsigned long& jitterTargetTime);

    /**
     * @brief Realiza o envio de pacotes de Log ou Monitoramento baseado em confirmações pendentes
     * Se um ACK ainda não foi recebido, tenta retransmitir o pacote correspondente
     * @param device Dispositivo que realizará o envio
     * @param st_monitoring Timer de controle de monitoramento
     * @param jitterTargetTime Tempo alvo calculado com jitter para evitar colisões
     * @param ackMonitoring Estado atual da confirmação de monitoramento
     * @param ackLog Estado atual da confirmação de log
     */
    void SendPacketLog(DeviceBase& device, SimpleTimer& st_monitoring, unsigned long& jitterTargetTime, bool ackMonitoring, bool ackLog);

    /**
     * @brief Ajusta dinamicamente os tempos de transmissão de pacotes safety baseados na proximidade de veículos
     * Altera o intervalo do timer conforme o nível de perigo (Level 1, 2 ou 3).
     * @param personal Referência ao dispositivo pessoal
     * @param st Timer que terá seu intervalo ajustado
     * @param hasTarget Se tiver recebido um pacote válido
     * @param level Nível de alerta atual
     * @param lastLevel Último nível de alerta registrado para detecção de mudança
     */
    void SendTime(PersonalDevice& personal, SimpleTimer& st, bool& hasTarget, int& level, int& lastLevel);

    /**
     * @brief Processa dados de localização recebidos
     * Calcula a distância relativa e atualiza a lista interna de veículos próximos.
     * @param personal Dispositivo local
     * @param id Identificador do dispositivo remoto
     * @param srcLat Latitude recebida
     * @param srcLng Longitude recebida
     */
    void ProcessData(PersonalDevice& personal, uint8_t id, double srcLat, double srcLng);

    /**
     * @brief Avalia e imprime no console alertas de proximidade baseados nos raios de segurança.
     * @param personal Referência ao dispositivo pessoal
     */
    void ActiveAlert(PersonalDevice& personal);

    /**
     * @brief Monitora e registra eventos críticos no sistema
     * Verifica mudanças de distância, nível de bateria, precisão HDOP e status do GPS para adicionar à lista de eventos
     * @param personal Referência ao dispositivo pessoal
     */
    void MonitoringEvent(PersonalDevice& personal);

private:
    uint8_t bateriaTeste = 100; // Variável de teste para simulação de bateria
    int index = 100;           // Índice auxiliar para controle
    SimpleTimer savePosTimer = SimpleTimer(30000); // Timer interno para persistência da posição
};

#endif