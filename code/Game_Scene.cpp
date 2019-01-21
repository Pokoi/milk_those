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
        { ID(nombre_ID),  "game-scene/nombre_archivo.extension"},
        { ID(udder),      "game-scene/udder.png"},

        //...

    };

    // Para determinar el número de items en el array textures_data, se divide el tamaño en bytes
    // del array completo entre el tamaño en bytes de un item:

    unsigned Game_Scene::textures_count = sizeof(textures_data) / sizeof(Texture_Data);

    // ---------------------------------------------------------------------------------------------
    // Definiciones de los atributos estáticos de la clase:

    //TODO: Implementar los atributos estáticos constantes de la clase
    //constexpr tipo_dato Game_Scene:: nombre_variable = valor;

    constexpr size_t Game_Scene:: bullet_amount = 10;
    constexpr float  Game_Scene:: bullet_speed  = 100f;
    constexpr float  Game_Scene:: milk_for_shot = 0.10f;


    // ---------------------------------------------------------------------------------------------

    Game_Scene::Game_Scene()
    {
        // Se establece la resolución virtual (independiente de la resolución virtual del dispositivo).
        // En este caso no se hace ajuste de aspect ratio, por lo que puede haber distorsión cuando
        // el aspect ratio real de la pantalla del dispositivo es distinto.

        canvas_width  = 1280;
        canvas_height =  720;

        aspect_ratio_adjusted = false;


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
            }
            else switch (event.id)
            {
                case ID(touch-started):     // El usuario toca la pantalla
                case ID(touch-moved):
                {
                    //Guardamos la posición tocada por el usuario
                    Point2f touch_location = { *event[ID(x)].as< var::Float > (),    //Coordenada X
                                               *event[ID(y)].as< var::Float > ()     //Coordenada Y
                                             };

                    // Si el índice es cero significa que no se ha pulsado ninguna ubre anteriormente

                    if(last_udder_clicked == 0){

                        if(first_udder_pointer -> contains(touch_location))
                        {
                            last_udder_clicked = 1;
                            spawn_bullet(first_spawn_position);
                        }

                        else if(second_udder_pointer -> contains(touch_location))
                        {
                            last_udder_clicked = 2;
                            spawn_bullet(first_spawn_position);
                        }
                    }

                    else if(last_udder_clicked == 1 && second_udder_pointer -> contains (touch_location))
                    {
                        last_udder_clicked = 2;
                        spawn_bullet(first_spawn_position);
                    }
                    else if(last_udder_clicked == 2 && first_udder_pointer  -> contains (touch_location))
                    {
                        last_udder_clicked = 1;
                        spawn_bullet(first_spawn_position);
                    }

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
                    case LOADING: render_loading   (*canvas); break;
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
        else
        if (timer.get_elapsed_seconds () > 1.f)         // Si las texturas se han cargado muy rápido
        {                                               // se espera un segundo desde el inicio de
            create_gameobjects();                       // la carga antes de pasar al juego para que
            restart_game   ();                          // el mensaje de carga no aparezca y desaparezca
                                                        // demasiado rápido.
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
        GameObject_Handle first_udder  (new GameObject (textures[ID(udder)]. get()));
        GameObject_Handle second_udder (new GameObject (textures[ID(udder)]. get()));
        GameObject_Handle timer_object (new GameObject (textures[ID(timer)]. get()));
        GameObject_Handle bucket       (new GameObject (textures[ID(bucket)].get()));

        first_udder  -> set_position({(canvas_width * 0.5f) - first_udder  -> get_width() , (first_udder  -> get_height() * 0.5f)});
        second_udder -> set_position({(canvas_width * 0.5f) + second_udder -> get_width() , (second_udder -> get_height() * 0.5f)});
        timer_object -> set_position({(canvas_width * 0.5f) , (canvas_height - (timer_object -> get_height() * 0.5f))});
        bucket       -> set_position({(canvas_width * 0.5f) , (canvas_height - (timer_object -> get_height() * 0.5f) - (bucket -> get_height() * 0.5f))});

        gameobjects.push_back(first_udder) ;
        gameobjects.push_back(second_udder);
        gameobjects.push_back(bucket)      ;

       // Se crean los proyectiles de leche
        for(unsigned iterator = 0; iterator < bullet_amount; iterator++)
        {
            GameObject_Handle milk_bullet (new GameObject (textures[ID(bullet)]. get()));

            milk_bullet -> hide ();

            bullets.    push_back(milk_bullet);
            gameobjects.push_back(milk_bullet);
        }



        // Se guardan punteros a los gameobjects que se van a usar frecuentemente:
        // nombre_puntero = nombre_objeto.get();

        first_udder_pointer  = first_udder .get();
        second_udder_pointer = second_udder.get();
        bucket_pointer       = bucket.      get();

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

        first_spawn_position  = {first_udder_pointer  -> get_position_x(), (first_udder_pointer  -> get_position_y() + (first_udder_pointer  -> get_height() + 0.5f))};
        second_spawn_position = {second_udder_pointer -> get_position_x(), (second_udder_pointer -> get_position_y() + (second_udder_pointer -> get_height() + 0.5f))};

        // Reseteamos el índice de última ubre pulsada
        last_udder_clicked = 0;

        gameplay = WAITING_TO_START;
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::start_playing ()
    {
        //TODO: Implementar las cosas que se tengan que realizar al empezar a jugar

        liters = 0f;

        gameplay = PLAYING;
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::run_simulation (float time)
    {
        // Se actualiza el estado de todos los gameobjects:

        for (auto & gameobject : gameobjects)
        {
            gameobject->update (time);
        }

        // Comprobación de colisión de los proyectiles:

        for (auto & bullet : bullets)
        {
            if(bullet -> is_visible())
            {
                // Si el proyectil ha llegado hasta el cubo,
                // Lo hacemos invisible y reseteamos su velocidad y posición
                // Sumamos los puntos adecuados

                if( bullet -> intersects( *bucket_pointer))
                {
                    bullet -> hide();
                    bullet -> set_speed({0,0});
                    bullet -> set_position({0,0});
                    liters += milk_for_shot;
                }
            }
        }

        //TODO: implementación de la IA

        //TODO: implementación de posibles colisiones
    }


    // ---------------------------------------------------------------------------------------------

    void Game_Scene::render_loading (Canvas & canvas)
    {
        //TODO: tiene que haber alguna textura con ID loading para la carga
        Texture_2D * loading_texture = textures[ID(loading)].get ();

        if (loading_texture)
        {
            canvas.fill_rectangle
            (
                { canvas_width * .5f, canvas_height * .5f },
                { loading_texture->get_width (), loading_texture->get_height () },
                  loading_texture
            );
        }
    }

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

        float real_aspect_ratio = float( context->get_surface_width () ) / context->get_surface_height ();

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
        for(iterator = 0; iterator < bullets.size() /* && bullets[iterator] -> is_visible */; iterator++)
        {};

        if(iterator < bullets.size())
        {
            //bullets[iterator] -> set_position(point);
            //bullets[iterator] -> set_speed({0, bullet_speed});
            //bullets[iterator] -> show();
        }

    }

}
