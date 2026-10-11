

struct CameraCommand
{
    float yaw_delta = 0.0f;   // [rad] - cambio de longitud, acumulado en el cuadro
    float pitch_delta = 0.0f; // [rad] - cambio de latitud
    float dist_delta = 0.0f;  // [unidades de escena] - (+) se aleja
};