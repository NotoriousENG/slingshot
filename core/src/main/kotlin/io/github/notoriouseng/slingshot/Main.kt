package io.github.notoriouseng.slingshot

import com.badlogic.gdx.ApplicationAdapter
import com.badlogic.gdx.Gdx
import com.badlogic.gdx.graphics.Texture
import com.badlogic.gdx.graphics.g2d.SpriteBatch
import com.badlogic.gdx.graphics.glutils.ShapeRenderer
import com.badlogic.gdx.graphics.Color
import com.badlogic.gdx.math.Vector2
import com.badlogic.gdx.Input
import com.badlogic.gdx.graphics.Cursor
import com.badlogic.gdx.graphics.g2d.BitmapFont
import com.badlogic.gdx.utils.Timer
import ktx.app.clearScreen
import kotlin.math.abs
import kotlin.math.max

fun clamp(value: Float, min: Float, max: Float): Float {
    return min.coerceAtLeast(value.coerceAtMost(max))
}

data class Shot(var pos: Vector2, var vel: Vector2)
data class Man(var pos: Vector2, var speed: Float)

/** [com.badlogic.gdx.ApplicationListener] implementation shared by all platforms. */
class Main : ApplicationAdapter() {
    private lateinit var spriteBatch: SpriteBatch
    private lateinit var shapeRenderer: ShapeRenderer
    private lateinit var texSlingshot: Texture
    private lateinit var texBall: Texture
    private lateinit var texMan: Texture
    private var position: Vector2 = Vector2(350.0f, 64.0f)
    private val speed = 300.0f
    private val shots = arrayListOf<Shot>()
    private val men = arrayListOf<Man>()
    private var spawnInterval = 2f // Spawn every 2 seconds
    private var slingshotActive = false
    private var score = 0
    private var hiscore = 0;
    private lateinit var font: BitmapFont

    override fun create() {
        spriteBatch = SpriteBatch()
        shapeRenderer = ShapeRenderer()
        texSlingshot = Texture(Gdx.files.internal("textures/slingshot.png"))
        texBall = Texture(Gdx.files.internal("textures/ball.png"))
        texMan = Texture(Gdx.files.internal("textures/man.png"))
        font = BitmapFont() // Default font provided by LibGDX
        // Schedule the spawn task
        Timer.schedule(object : Timer.Task() {
            override fun run() {
                spawnMan()
            }
        }, 0f, spawnInterval) // Delay: 0s, Repeat: every spawnInterval seconds

        // Schedule a task to decrease the spawn interval every 10 seconds
        Timer.schedule(object : Timer.Task() {
            override fun run() {
                spawnInterval = maxOf(spawnInterval * .75f, 0.01f)
                // Update the spawn task with the new interval
                Timer.instance().clear()
                Timer.schedule(object : Timer.Task() {
                    override fun run() {
                        spawnMan()
                    }
                }, 0f, spawnInterval)
            }
        }, 10f, 10f) // Delay: 10s, Repeat: every 10 seconds

    }

    private fun spawnMan() {
        // Add a new object at a random position (example: within screen bounds)
        val x = (Math.random() * Gdx.graphics.width - 32.0f).toFloat()
        val y = 900.0f
        men.add(Man(Vector2(x, y), -(48.0f + Math.random().toFloat() * 100.0f)))
    }

    override fun render() {
        val delta = Gdx.graphics.deltaTime
        update(delta)
        draw()
    }

    private fun update(delta: Float) {
        Gdx.graphics.setSystemCursor(Cursor.SystemCursor.Arrow)
        val screenHeight = Gdx.graphics.getHeight().toFloat();

        val slingshotOrigin = Vector2(348.0f, 100.0f);
        val ballStart = slingshotOrigin.cpy().sub(Vector2(0.0f, 32.0f));
        val mouse = Vector2(Gdx.input.x.toFloat(), screenHeight - Gdx.input.y.toFloat())

        if (!this.slingshotActive) {
            this.position = ballStart.cpy()
            if (mouse.dst(ballStart) <= 32.0f) {
                Gdx.graphics.setSystemCursor(Cursor.SystemCursor.Hand)
                if (Gdx.input.isButtonJustPressed(Input.Buttons.LEFT)){
                    this.slingshotActive = true
                }
            }
        } else {
            Gdx.graphics.setSystemCursor(Cursor.SystemCursor.Hand)
            this.position.x = mouse.x - 16.0f
            this.position.y = mouse.y - 16.0f
            this.position.x = clamp(this.position.x, -16.0f, 816.0f)
            this.position.y = clamp(this.position.y, -16.0f, 616.0f)

            if (!Gdx.input.isButtonPressed(Input.Buttons.LEFT)) {
                val dir = slingshotOrigin.cpy().sub(this.position).nor()
                val fac = abs(slingshotOrigin.cpy().dst(this.position)) * 10.0f
                val shot = Shot(this.position.cpy(), dir.cpy().scl(fac))
                this.shots.add(shot)
                this.slingshotActive = false;
            }
        }


        updateShots(delta)
        updateMen(delta)
    }

    private fun updateShots(delta: Float) {
        val screenWidth = Gdx.graphics.getWidth().toFloat();
        val screenHeight = Gdx.graphics.getHeight().toFloat();
        val iterator = shots.iterator()
        val gravity = -980f
        while (iterator.hasNext()) {
            val shot = iterator.next()
            // Apply gravity to the vertical direction
            shot.vel.y += gravity * delta
            shot.pos.add(shot.vel.cpy().scl(delta))
            val dim = 32.0f;
            if (shot.pos.x > screenWidth + dim || shot.pos.x < -dim || shot.pos.y > screenHeight + dim || shot.pos.y < -dim) {
                iterator.remove()
            }
        }
    }

    private fun updateMen(delta: Float) {
        val iterator = men.iterator()
        while (iterator.hasNext()) {
            val man = iterator.next()
            // Apply gravity to the vertical direction
            man.pos.y += man.speed * delta
            if (man.pos.y < -120.0f) {
                iterator.remove()
                this.hiscore = max(this.score, this.hiscore)
                this.score = 0
                men.clear()
                break
            }
            val shotIterator = shots.iterator()
            while (shotIterator.hasNext()) {
                val shot = shotIterator.next()
                if (shot.pos.dst(man.pos) <= 64.0f) {
                    //shotIterator.remove()
                    iterator.remove()
                    score++
                    break
                }
            }
        }
    }

    private fun draw() {
        clearScreen(0f, 0f, 0f, 1f)
        spriteBatch.begin()
        spriteBatch.draw(texSlingshot, 300.0f, 000.0f)
        spriteBatch.draw(texBall, this.position.x, this.position.y)

        for (shot in this.shots) {
            spriteBatch.draw(texBall, shot.pos.x, shot.pos.y)
        }
        for (man in this.men) {
            spriteBatch.draw(texMan, man.pos.x, man.pos.y)
        }

        font.draw(spriteBatch, "Score: ${this.score}", 40f, 590f)
        if (this.hiscore > 0) {
            font.draw(spriteBatch, "Hi-Score: ${this.hiscore}", 680f, 590f)
        }

        spriteBatch.end()

        // slingshot strings
        shapeRenderer.begin(ShapeRenderer.ShapeType.Line)
        shapeRenderer.color = Color.WHITE
        shapeRenderer.line(310.0f, 100.0f, this.texBall.width/2 + this.position.x, this.texBall.height/2 +this.position.y)
        shapeRenderer.line(418.0f, 100.0f, this.texBall.width/2 + this.position.x, this.texBall.height/2 +this.position.y)
        shapeRenderer.end()
    }

    override fun dispose() {
        texSlingshot.dispose()
        texBall.dispose()
        texMan.dispose()
        font.dispose()
        spriteBatch.dispose()
        shapeRenderer.dispose()
    }
}
