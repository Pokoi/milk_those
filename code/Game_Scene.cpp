/*
 * GAME SCENE
 * Copyright © 2018+ Ángel Rodríguez Ballesteros
 *
 * Distributed under the Boost Software License, version  1.0
 * See documents/LICENSE.TXT or www.boost.org/LICENSE_1_0.txt
 *
 * angel.rodriguez@esne.edu
 */

/*
 * MODIFIED BY
 *
 * Jesus 'Pokoi' Villar
 * © pokoidev 2019 (pokoidev.com)
 *
 * Creative Commons License:
 * Attribution 4.0 International (CC BY 4.0)
 *
 */

#include "Game_Scene.hpp"
#include "Menu_Scene.hpp"
#include "Final_Scene.hpp"

#include <cstdlib>
#include <basics/Canvas>
#include <basics/Director>

using namespace basics;
using namespace std;

namespace project_template
{
    // ---------------------------------------------------------------------------------------------
    // ID y ruta de las texturas que se deben cargar para esta escena.

    //TODO: Implementar las texturas con su ID y ruta
    Game_Scene::Texture_Data Game_Scene::textures_data[] =
    {
        { ID(left_udder),       "game-scene/udder_left.png"},
        { ID(right_udder),      "game-scene/udder_right.png"},
        { ID(bucket),           "game-scene/bucket.png"},
        { ID(bullet),           "game-scene/bullet.png"},
        { ID(pausa),            "game-scene/pausa-button.png"},
        { ID(pausa_text),       "game-scene/pausa_text.png"},


    };

    // Para determinar el número de items en el array textures_data, se divide el tamaño en bytes
    // del array completo entre el tamaño en bytes de un item:

    unsigned Game_Scene::textures_count = sizeof(textures_data) / sizeof(Texture_Data);

    // ---------------------------------------------------------------------------------------------
    // Definiciones de los atributos estáticos de la clase:

    //TODO: Implementar los atributos estáticos constantes de la clase
    //constexpr tipo_dato Game_Scene:: nombre_variable = valor;

    constexpr size_t Game_Scene:: bullet_amount;
    constexpr float  Game_Scene:: bullet_speed;
    constexpr float  Game_Scene:: milk_for_shot;
    constexpr int    Game_Scene:: max_time;
    float  Game_Scene:: liters;


    // ---------------------------------------------------------------------------------------------

    Game_Scene::Game_Scene()
    {
        // Se establece la resolución virtual (independiente de la resolución virtual del dispositivo).
        // En este caso no se hace ajuste de aspect ratio, por lo que puede haber distorsión cuando
        // el aspect ratio real de la pantalla del dispositivo es distinto.

        canvas_width  =  720;
        canvas_height =  1280;

        aspect_ratio_adjusted = false;
        game_paused = false;


        // Se inicia la semilla del generador de números aleatorios:
        srand (unsigned(time(nullptr)));

        // Se inicializan otros atributos:

        initialize ();
    }

    // ---------------------------------------------------------------------------------------------
    // Algunos atributos se inicializan en este método en lugar de hacerlo en el constructor porque
    // este método puede ser llamado más veces para restablecer el estado de la escena y el constructor
    // solo se invoca una vez.

    bool Game_Scene::initialize ()
    {
        state     = LOADING;
        suspended = true;
        gameplay  = UNINITIALIZED;

        return true;
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::suspend ()
    {
        suspended = true;               // Se marca que la escena ha pasado a primer plano
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::resume ()
    {
        suspended = false;              // Se marca que la escena ha pasado a segundo plano
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::handle (Event & event)
    {
        if (state == RUNNING)               // Se descartan los eventos cuando la escena está LOADING
        {
            if (gameplay == WAITING_TO_START)
            {
                start_playing ();           // Se empieza a jugar cuando el usuario toca la pantalla
                                            // por primera vez

                timer.reset();
            }
            else switch (event.id)
            {
                case ID(touch-started):
                {
                    //Guardamos la posición tocada por el usuario
                    Point2f touch_location = { *event[ID(x)].as< var::Float > (),    //Coordenada X
                                               *event[ID(y)].as< var::Float > ()     //Coordenada Y
                    };

                    if(game_paused)
                    {
                        //timer.resume_timer();
                        game_paused = false;
                        pausa_button_pointer -> show();
                        pausa_text_pointer -> hide();
                        pause_the_game(game_paused);


                    }
                    else
                    {

                        //Si el índice es cero significa que no se ha pulsado ninguna ubre anteriormente

                        if (last_udder_clicked == 0) {

                            if (first_udder_pointer->contains(touch_location)) {
                                last_udder_clicked = 1;
                                spawn_bullet(first_spawn_position);
                            } else if (second_udder_pointer->contains(touch_location)) {
                                last_udder_clicked = 2;
                                spawn_bullet(second_spawn_position);
                            }
                        } else if (last_udder_clicked == 1 &&
                                   second_udder_pointer->contains(touch_location)) {
                            last_udder_clicked = 2;
                            spawn_bullet(second_spawn_position);
                        } else if (last_udder_clicked == 2 &&
                                   first_udder_pointer->contains(touch_location)) {
                            last_udder_clicked = 1;
                            spawn_bullet(first_spawn_position);
                        }

                        if(pausa_button_pointer->contains(touch_location))
                        {
                            //timer.stop_timer();
                            game_paused = true;
                            pausa_button_pointer -> hide();
                            pausa_text_pointer -> show();
                            pause_the_game(game_paused);

                        }
                    }

                    break;
                }
                case ID(touch-moved):
                {
                    break;
                }

                case ID(touch-ended):       // El usuario deja de tocar la pantalla
                {

                    break;
                }
            }
        }
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::update (float time)
    {
        if (!suspended) switch (state)
        {
            case LOADING: load_textures  ();     break;
            case RUNNING: run_simulation (time); break;
            case ERROR:   break;
        }
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::render (Context & context)
    {
        if (!suspended)
        {
            // El canvas se puede haber creado previamente, en cuyo caso solo hay que pedirlo:

            Canvas * canvas = context->get_renderer< Canvas > (ID(canvas));

            // Si no se ha creado previamente, hay que crearlo una vez:

            if (!canvas)
            {
                 canvas = Canvas::create (ID(canvas), context, {{ canvas_width, canvas_height }});
            }

            // Si el canvas se ha podido obtener o crear, se puede dibujar con él:

            if (canvas)
            {
                canvas->clear ();

                switch (state)
                {
                    case LOADING: ; break;
                    case RUNNING: render_playfield (*canvas); break;
                    case ERROR:   break;
                }
            }
        }
    }

    // ---------------------------------------------------------------------------------------------
    // En este método solo se carga una textura por fotograma para poder pausar la carga si el
    // juego pasa a segundo plano inesperadamente. Otro aspecto interesante es que la carga no
    // comienza hasta que la escena se inicia para así tener la posibilidad de mostrar al usuario
    // que la carga está en curso en lugar de tener una pantalla en negro que no responde durante
    // un tiempo.

    void Game_Scene::load_textures ()
    {
        if (textures.size () < textures_count)          // Si quedan texturas por cargar...
        {
            // Las texturas se cargan y se suben al contexto gráfico, por lo que es necesario disponer
            // de uno:

            Graphics_Context::Accessor context = director.lock_graphics_context ();

            if (context)
            {
                // Se ajusta el aspect ratio si este no se ha ajustado
                if(!aspect_ratio_adjusted){

                    adjust_aspect_ratio(context);
                }

                // Se carga la siguiente textura (textures.size() indica cuántas llevamos cargadas):

                Texture_Data   & texture_data = textures_data[textures.size ()];
                Texture_Handle & texture      = textures[texture_data.id] = Texture_2D::create (texture_data.id, context, texture_data.path);

                // Se comprueba si la textura se ha podido cargar correctamente:

                if (texture) context->add (texture); else state = ERROR;

                // Cuando se han terminado de cargar todas las texturas se pueden crear los gameobjects que
                // las usarán e iniciar el juego:
            }
        }
        else {

            create_gameobjects();
            restart_game();

            state = RUNNING;
        }

    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::create_gameobjects()
    {

        //TODO: crear y configurar los gameobjects de la escena
        // 1) Se crean y configuran los gameobjects:

        //GameObject_Handle  nombre_objeto(new GameObject (textures[ID(nombre_ID)].get() ));
        //...

        // 2) Se establecen los anchor y position de los GameObject
        // Si no se especifica se colocan por defecto en la posición (0,0) y el anchor en el centro
        //nombre_objeto->set_anchor (...);
        //nombre_objeto->set_position ({coordenada_x, coordenada_y});

        // 3) Se añaden a la lista de game objects
        //gameobjects.push_back (nombre_objeto);


        // Se crean los objetos no dinámicos de la escena
        GameObject_Handle first_udder  (new GameObject (textures[ID(left_udder)].  get(), real_aspect_ratio));
        GameObject_Handle second_udder (new GameObject (textures[ID(right_udder)]. get(), real_aspect_ratio));
        GameObject_Handle bucket       (new GameObject (textures[ID(bucket)].      get(), real_aspect_ratio));
        GameObject_Handle pausa_button (new GameObject (textures[ID(pausa)].       get(), real_aspect_ratio));
        GameObject_Handle pausa_signal (new GameObject (textures[ID(pausa_text)].  get(), real_aspect_ratio));

        first_udder  -> set_position({(canvas_width * 0.5f) - (first_udder  -> get_width() * 0.5f) , (canvas_height - first_udder  -> get_height() * 0.5f)});
        second_udder -> set_position({(canvas_width * 0.5f) + (second_udder -> get_width() * 0.5f) , (canvas_height - second_udder -> get_height() * 0.5f)});
        bucket       -> set_position({(canvas_width * 0.5f) , ((bucket -> get_height() * 0.5f))});
        pausa_button -> set_position({pausa_button -> get_width() * 0.5f + (pausa_button -> get_width()), (canvas_height - pausa_button -> get_height())});
        pausa_signal -> set_position({canvas_width * 0.5f, canvas_height * 0.5f});

        gameobjects.push_back(first_udder) ;
        gameobjects.push_back(second_udder);
        gameobjects.push_back(bucket)      ;
        gameobjects.push_back(pausa_button);
        gameobjects.push_back(pausa_signal);

       // Se crean los proyectiles de leche
        for(unsigned iterator = 0; iterator < bullet_amount; iterator++)
        {
            GameObject_Handle milk_bullet (new GameObject (textures[ID(bullet)]. get(), real_aspect_ratio));

            milk_bullet -> hide ();

            bullets.    push_back(milk_bullet);
            gameobjects.push_back(milk_bullet);
        }



        // Se guardan punteros a los gameobjects que se van a usar frecuentemente:
        // nombre_puntero = nombre_objeto.get();

        first_udder_pointer  = first_udder .get();
        second_udder_pointer = second_udder.get();
        bucket_pointer       = bucket.      get();
        pausa_button_pointer = pausa_button.get();
        pausa_text_pointer   = pausa_signal.get();

    }

    // ---------------------------------------------------------------------------------------------
    // Cuando el juego se inicia por primera vez o cuando se reinicia porque un jugador pierde, se
    // llama a este método para restablecer los gameobjects:

    void Game_Scene::restart_game()
    {
        // TODO: resetear valores iniciales de los gameobjects que lo requieran
        //nombre_objeto->nombre_metodo(parametros);

        // Reseteamos la visibilidad y posicion de todos los proyectiles

        for (auto & gameobject : bullets)
        {
            gameobject -> set_position({0,0});
            gameobject -> hide();
        }

        // Establecemos la posición de los spawns en función de la posición de las ubres

        first_spawn_position  = {first_udder_pointer  -> get_position_x(), first_udder_pointer  -> get_bottom_y()};
        second_spawn_position = {second_udder_pointer -> get_position_x(), second_udder_pointer -> get_bottom_y()};

        // Reseteamos el índice de última ubre pulsada
        last_udder_clicked = 0;

        liters = 0.0f;

        pausa_button_pointer -> show();
        pausa_text_pointer -> hide();

        gameplay = WAITING_TO_START;
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::start_playing ()
    {

        gameplay = PLAYING;
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::run_simulation (float time)
    {

        if(gameplay != ENDING) {

            // Se actualiza el estado de todos los gameobjects:

            for (auto &gameobject : gameobjects) {
                gameobject->update(time);
            }

            // Comprobación de colisión de los proyectiles:

            for (auto &bullet : bullets) {
                if (bullet->is_visible()) {
                    // Si el proyectil ha llegado hasta el cubo,
                    // Lo hacemos invisible y reseteamos su velocidad y posición
                    // Sumamos los puntos adecuados

                    if (bullet->get_position_y() <= (0.75f * bucket_pointer->get_height())) {
                        bullet->hide();
                        bullet->set_speed({0, 0});
                        bullet->set_position({0, 0});
                        liters += milk_for_shot;
                    }

                }
            }

            // Comprobación de tiempo restante de la partida:

            if (timer.get_elapsed_seconds() >= max_time) {
                gameplay = ENDING;
            }
        }
        else
        {

            director.run_scene (shared_ptr< Scene >(new Final_Scene(liters)));
        }


    }


    // ---------------------------------------------------------------------------------------------


    // ---------------------------------------------------------------------------------------------
    // Simplemente se dibujan todos los gameobjects que conforman la escena.

    void Game_Scene::render_playfield (Canvas & canvas)
    {
        for (auto & gameobject : gameobjects)
        {
            gameobject->render (canvas);
        }
    }

    // ---------------------------------------------------------------------------------------------
    // Ajusta el aspect ratio

    void Game_Scene::adjust_aspect_ratio(Context & context)
    {

        real_aspect_ratio = float( context->get_surface_width () ) / context->get_surface_height ();

        // TODO: Elegir qué disposición se desea mantener

        // Si se desea mantener el alto y ajustar el ancho (disposición horizontal)
        //canvas_width = unsigned( canvas_height * real_aspect_ratio );

        // Si se desea mantener el ancho y ajustar el alto (disposición vertical)
        canvas_height = unsigned ( canvas_width * real_aspect_ratio);

        aspect_ratio_adjusted = true;
    }

    // ---------------------------------------------------------------------------------------------
    // Spawnea un proyectil en la posición dada y se le aplica una velocidad

    void Game_Scene::spawn_bullet (const Point2f & point)
    {
        unsigned iterator;
        for(iterator = 0; iterator < bullets.size()  && bullets[iterator] -> is_visible(); iterator++)
        {};

        if(iterator < bullets.size())
        {
            bullets[iterator] -> set_position(point);
            bullets[iterator] -> set_speed({0, bullet_speed});
            bullets[iterator] -> show();
        }

    }

    // ---------------------------------------------------------------------------------------------
    // Pausa el juego

    void Game_Scene::pause_the_game(bool paused) {

            for (auto bullet: bullets){

                if (bullet -> is_visible()){

                    if(paused) bullet -> set_speed({0,0});
                    else       bullet -> set_speed({0, bullet_speed});
                    }
            }

     }

}


