#ifndef PERSONALDEVICE_H
#define PERSONALDEVICE_H

#include "DeviceBase.h"

/**
 * @brief Classe que representa um dispositivo pessoal
 */
class PersonalDevice : public DeviceBase {
public:
    /**
     * @brief Construtor da classe PersonalDevice.
     * Define o tipo de dispositivo como PERSONAL_DEVICE.
     */
    PersonalDevice();

    /**
     * @brief Determina o nível de urgência de envio baseado na distância mínima de um veículo
     * @param minDistance Distância do veículo mais próximo.
     * @return int Nível de validade (1: Crítico, 2: Alerta, 3: Normal)
     */
    int isValidSend(double minDistance);

    /**
     * @brief Atualiza a lista de veículos próximos ou adiciona um novo se não existir
     * @param id Identificador do veículo detectado.
     * @param dist Distância calculada até o veículo.
     */
    void updateVehicleList(uint8_t id, double dist);

    /**
     * @brief Remove veículos da lista que não enviam sinais há mais de 12 segundos.
     */
    void cleanOldVehicles();

    /**
     * @brief Salva a posição GPS atual no histórico das últimas 5 posições
     * Realiza o o deslocamento dos dados caso o array esteja cheio
     */
    void saveLastPosition();

protected:
    /** @brief Constrói o pacote de segurança ignorando velocidade e curso (fixos em 0.0) */
    void buildSafetyPacket() override;

    /** @brief Gera um ID aleatório de pacote e constrói o pacote de monitoramento de status */
    void buildMonitoringPacket() override;

    /** @brief Gera um ID aleatório de pacote e constrói o pacote de logs incluindo histórico e veículos próximos. */
    void buildLogPacket() override;

    /** @brief Executa ações ao decodificar um pacote recebido, como exibir dados no Serial. */
    void onReceiveDecoded() override;

private:
    /** @brief Armazena o último tempo de reset em milissegundos. */
    uint32_t lastMinResetMs = 0;
};

#endif